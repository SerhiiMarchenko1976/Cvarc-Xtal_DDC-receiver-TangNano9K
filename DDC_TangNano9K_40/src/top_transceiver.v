module top_trx(
	input sys_clk,            // ВХОД 27 МГц со штатного кварца Tang Nano 9K (Пин 52)
	input start,

	// DAC_ADC interface
	inout [13:0] data_io,
	output adc_clk,
	output dac_clk,
	output adc_off,        // to pin OEB AD9226
	output reg dac_pwm,

	// SPI bus
	input  SPI_MOSI,
	output SPI_MISO,
	input  SPI_SS,
	input  SPI_SCK,

	// control
	input n_ptt,              // входы строго с подтяжкой к +3,3v
	input n_tune,
	input n_cw_key,
	output reg preamp_on,
	output reg att_on,
	output [4:0] filter,      
	output test
);

	wire [13:0] dac_data;

	assign data_io = n_ptt ? 14'bZ : dac_data;
	assign adc_off = ~n_ptt;
	wire reset;
	wire decim;
	reg [1:0] s_rate;
	
	// 16-битные шины квадратур под оптимизированный Receiver.v
	wire signed [15:0] rx_real, rx_imag;
	
	// ОПТИМИЗИРОВАНО ПОД 40 БИТ: Все управляющие шины сужены до 40 бит (5 байт)
	wire [39:0] control;
	wire [39:0] rxFromFifo;
	reg  [39:0] rxToFifo;
	reg  [11:0] reg_adc_data;
	wire spi_done;
	reg  [31:0] rx_freq;
	reg  [31:0] tx_freq;
	reg signed [15:0] tx_real, tx_imag;

	// СЕТЬ СИНХРОННЫХ ЧАСТОТ НА БАЗЕ ЕДИНОЙ ЧАСТОТЫ 60 МГц
	wire clk_60m;       // ЕДИНСТВЕННАЯ базовая частота 60.00 МГц для всего SDR-тракта
	wire clk_10m;       // Служебная частота 10.00 МГц (60.00 / 6) напрямую из PLL
	wire clk_21m;       // Дополнительный выход PLL
	reg  clk_96k = 0;   // Звуковая частота дискретизации (10.00 МГц / 104)

	// Аппаратное тактирование внешних микросхем от единого клока 60 МГц
	assign adc_clk = n_ptt ? clk_60m  : 1'b0;  
	assign dac_clk = n_ptt ? 1'b0     : clk_60m; 
	assign reset = start;

	// 1. Аппаратный PLL: Вход 27 МГц -> Выход 60 МГц и аппаратные 10 МГц на clkoutd
	Gowin_rPLL pll_inst (
		.clkout(clk_60m),
		.clkoutd(clk_10m),
		.clkoutd3(clk_21m),
		.clkin(sys_clk)
	);

	// 2. Синхронный делитель на 104 (10 МГц / 104 = ~96.15 кГц)
	reg [5:0] div_counter = 0;
	always @(posedge clk_10m) begin
		if (div_counter == 6'd51) begin 
			div_counter <= 0;
			clk_96k <= ~clk_96k;       
		end else begin
			div_counter <= div_counter + 6'd1;
		end
	end

	// Захват данных АЦП идет на частоте 60 МГц
	always @(posedge clk_60m) begin
		if (n_ptt) begin
			reg_adc_data <= data_io[11:0];
		end
	end

	// Модуль ДПФ (работают на аппаратных 10 МГц)
	filter filter_inst(
		.clock(clk_10m), 
		.frequency(rx_freq), 
		.selected_filter(filter)
	);

	// Приёмный тракт SDR на частоте 60 МГц
	assign test = decim;
	Receiver rx (
		.clock(clk_60m), 
		.reset(!reset),
		.adc_data(reg_adc_data),
		.rx_freq(rx_freq),
		.rx_real(rx_real), 
		.rx_imag(rx_imag),
		.s_rate(s_rate), 
		.decim(decim)
	);

	//------------------------------------------------------------------------------
	// СИНХРОНИЗАЦИЯ И РАЗБОР КОМАНД SPI (Карта регистров 40 бит)
	//------------------------------------------------------------------------------
	wire _spi_done;
	
	// Синхронизация одиночного флага spi_done (Безопасно для трассировки)
	cdc_sync #(1) done(.siga(spi_done), .rstb(1'b0), .clkb(clk_10m), .sigb(_spi_done));

	// ИСПРАВЛЕНО НА 40 БИТ: Бесплатный разворот Endianness для 5 байт напрямую из control
	wire [39:0] v_cmd = {control[7:0], control[15:8], control[23:16], control[31:24], control[39:32]};

	// Регистровый селектор: Адрес команды сидит в старшем байте v_cmd[39:32]
	always @(posedge clk_10m) begin
		if (_spi_done) begin
			case (v_cmd[39:32])
				
				8'h01: begin // КОМАНДА 01: Частота приёма rx_freq (все 32 бита данных)
					rx_freq <= v_cmd[31:0];
				end
				
				8'h02: begin // КОМАНДА 02: Периферия (читаем из младших фиксированных бит)
					dac_level  <= v_cmd[7:0]; 
					att_on     <= v_cmd[8];   // Выделен четкий одиночный бит
					preamp_on  <= v_cmd[9];   // Выделен четкий одиночный бит
					s_rate     <= v_cmd[11:10];
				end
				
				8'h03: begin // КОМАНДА 03: Поток квадратур звука передачи микрофона
					tx_real    <= v_cmd[15:0];
					tx_imag    <= v_cmd[31:16];
				end
				
				8'h04: begin // КОМАНДА 04: Частота передачи tx_freq (все 32 бита данных)
					tx_freq    <= v_cmd[31:0];
				end
                				8'h04: begin // КОМАНДА 04: Частота передачи tx_freq
					tx_freq    <= v_cmd[31:0];
				end

				8'h05: begin // НОВАЯ КОМАНДА 05: dac_level
					dac_level  <= v_cmd[7:0]; 
				end

				8'h06: begin // НОВАЯ КОМАНДА 06: Ширина спектра
					s_rate     <= v_cmd[1:0];
				end

                8'h07: begin // НОВАЯ КОМАНДА 07: att_on и preamp_on
					att_on     <= v_cmd[0];   // Выделен четкий одиночный бит
					preamp_on  <= v_cmd[1];   // Выделен четкий одиночный бит
				end
				
				

				
				default: begin
					// Игнорируем пустые или холостые циклы чтения (например, команду h00)
				end
				
			endcase
		end
	end

	// Передающий тракт SDR работает на частоте 60 МГц
	Transmitter tx (
		.clock(clk_60m), 
		.reset(!reset),
		.dac_data(dac_data),
		.tx_freq(tx_freq),
		.tx_real(tx_real),
		.tx_imag(tx_imag)
	);

	// ИСПРАВЛЕНО НА 40 БИТ: Плотная упаковка 16-битного звука и флагов в FIFO
	always @(posedge clk_10m) begin
		if (decim) begin
			rxToFifo <= {~n_ptt, ~n_tune, ~n_cw_key, 5'b0, rx_real[15:0], rx_imag[15:0]};
		end
	end

	// SPI Slave модуль переопределен на ширину 40 бит (5 байт)
	spi_slave #(.WIDTH(40)) spi(
		.rstb(reset),              
		.ten(reset),
		.tdata({rxFromFifo[7:0], rxFromFifo[15:8], rxFromFifo[23:16], rxFromFifo[31:24], rxFromFifo[39:32]}),
		.mlb(1'b1),
		.ss(SPI_SS),                   
		.sck(SPI_SCK),                
		.sdin(SPI_MOSI),               
		.sdout(SPI_MISO),              
		.done(spi_done),               
		.rdata(control)                
	);

	// Полностью синхронное тактирование FIFO
	fifo fifo_main(
		.Data(rxToFifo),     
		.WrReset(!reset),    
		.RdReset(!reset),    
		.WrClk(clk_10m),     
		.RdClk(clk_10m),     
		.WrEn(decim & reset), 
		.RdEn(_spi_done),     
		.Q(rxFromFifo),      
		.Empty(),            
		.Full()              
	);
	
	// Контроллер уровня мощности (PWM) на частоте 10 МГц
	reg [7:0] dac_level;
	reg [7:0] pwm_cnt;
	always @(posedge clk_10m) begin
		if (pwm_cnt >= dac_level) dac_pwm <= 0;
		else dac_pwm <= 1;
		pwm_cnt <= pwm_cnt + 1'd1;
	end

endmodule
