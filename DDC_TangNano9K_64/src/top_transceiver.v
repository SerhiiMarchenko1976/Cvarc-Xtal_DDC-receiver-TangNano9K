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


    // ОПТИМИЗИРОВАНО 64 БИТ:
    // 24-битные шины квадратур под оптимизированный Receiver.v
    //wire signed [23:0] rx_real, rx_imag;
    wire [63:0] control;
    wire [63:0] rxFromFifo;
    reg  [63:0] rxToFifo;
    reg  [11:0] reg_adc_data;
    wire spi_done;
    reg  [31:0] rx_freq;
    reg [31:0] tx_freq;
    reg signed [15:0]tx_real,tx_imag;
	

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


    //*************************************************************************
	/*команды управления из MCU в FPGA (слово 64 bit)
        Режим приема:
		control[43:36] - уровень выхода DAC (слово управления dac_pwm)
		control[35] - управление входным аттенюатором on/off
      control[34] - увч on/off
		control[33:32] - частота дискретизации
							  (00->24kHz 01->48kHz 10->96kHz)
		control[31:0]  - слово частоты приема (не фактическая, а частота начала спектра на экране)
        -------------------------------------------------------
        Режим передачи:
      control[63:32] - частота передачи
      control[31:0] - real:imag из MCU
	*/
	//*************************************************************************
	// СИНХРОНИЗАЦИЯ И РАЗБОР КОМАНД SPI (64 БИТА)
	//------------------------------------------------------------------------------

    wire _spi_done;

	// Оставляем синхронизацию только для ОДНОГО бита флага готовности (это бесплатно для ПЛИС)
	cdc_sync #(1) done(.siga(spi_done), .rstb(1'b0), .clkb(clk_10m), .sigb(_spi_done));

	// Аппаратный разворот Endianness на уровне проводов берем НАПРЯМУЮ из шины control
	
	wire [63:0] v_cmd = {control[7:0], control[15:8], control[23:16], control[31:24],
						 control[39:32], control[47:40], control[55:48], control[63:56]};

	// Обработка команд строго по системному клоку clk_10m и флагу _spi_done
	always @(posedge clk_10m) begin
		if (_spi_done) begin
			if (n_ptt) begin
				dac_level  <= v_cmd[43:36];
				att_on     <= v_cmd[35];
				preamp_on  <= v_cmd[34];
				s_rate     <= v_cmd[33:32];
				rx_freq    <= v_cmd[31:0];
				tx_freq    <= 32'b0;
			end else begin
				tx_real    <= v_cmd[31:16];
				tx_imag    <= v_cmd[15:0];
				tx_freq    <= v_cmd[63:32];
			end
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



	// ИСПРАВЛЕНО НА 64 БИТ: Плотная упаковка 16-битного звука и флагов в FIFO        
    // Упаковка данных в FIFO (частота clk_10m)
            always @(posedge clk_10m) begin
                    if (decim) begin
                        rxToFifo <= {~n_ptt, ~n_tune, ~n_cw_key, 13'b0, rx_real, 16'b0, rx_imag};
                        //rxToFifo <= {~n_ptt, ~n_tune, ~n_cw_key, 13'b0, rx_real[15:0], 16'b0, rx_imag[15:0]};//40
                        //rxToFifo <= {~n_ptt,~n_tune,~n_cw_key,rx_real,rx_imag};//23rx_real 23 rx_imag
                        
                    end
            end



	// SPI module, slave, mode3 (Шина 'control' должна удерживать данные до следующей посылки)
	spi_slave  #(.WIDTH(64)) spi(
		.rstb(reset),              
		.ten(reset),
		.tdata({rxFromFifo[7:0],   rxFromFifo[15:8],  rxFromFifo[23:16], rxFromFifo[31:24],
				rxFromFifo[39:32], rxFromFifo[47:40], rxFromFifo[55:48], rxFromFifo[63:56]}),
		.mlb(1'b1),
		.ss(SPI_SS),                   
		.sck(SPI_SCK),                
		.sdin(SPI_MOSI),               
		.sdout(SPI_MISO),              
		.done(spi_done),               
		.rdata(control)                
	);


	fifo fifo_main(
		.Data(rxToFifo),    // input [63:0] Data
		.WrReset(!reset),   // input WrReset
		.RdReset(!reset),   // input RdReset
		.WrClk(clk_10m),    // ИСПРАВЛЕНО: Системный тактовый сигнал вместо строба
		.RdClk(clk_10m),    // ИСПРАВЛЕНО: Системный тактовый сигнал вместо строба
		.WrEn(decim & reset), // ИСПРАВЛЕНО: Строб decim управляет разрешением записи
		.RdEn(_spi_done),   // ИСПРАВЛЕНО: Строб _spi_done управляет разрешением чтения
		.Q(rxFromFifo),     // output [63:0] Q
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

