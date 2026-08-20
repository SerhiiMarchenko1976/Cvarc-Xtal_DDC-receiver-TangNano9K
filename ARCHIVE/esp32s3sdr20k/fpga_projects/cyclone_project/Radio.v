
//////////////////////////////////////////////////////////////////////////////////

module radio(
   // Clock
	input clock_61M,			
	// ADC interface		
	input signed [11:0] adc_data,

	// DAC interface
	output [13:0] dac_data,
	output dac_clk,
	output reg level_pwm,
	// SPI bus
	input  SPI_MOSI,
	output SPI_MISO,
	input  SPI_SS,
	input  SPI_SCK,

	//control(строго с сильной подтяжкой к 3,3в)
	input n_tune,				//
	input n_cw_key,			//
	input n_ptt,				//
	input  [4:0]n_buttons_l,//
	input  [7:0]n_buttons_r,//
	
	//control BPF
	output [4:0]filter,// b10000-1..2MHz,
							 // b01000-2..4MHz,
							 // b00100-4..8MHz,
							 // b00010-8..16MHz,
							 // b00001-16..30MHz
	
	output [5:0]p_att,
	input start,
	output led,
	output test
	);
	 assign led =~ptt;
	 assign reset = start;
	 wire reset;
    wire pwm_CLK,clock_122M,clock_100k ,decim;
	 assign dac_clk = clock_122M;
	 reg [7:0] s_rate = 8'b10;
	 wire signed [23:0] rx_real, rx_imag;
	 wire fifo_full;
	 wire [63:0] control;
	 wire [63:0] rxFromFifo;
	 reg  [63:0] rxToFifo;
	 reg  [11:0] reg_adc_data;
	 wire spi_done;
	 reg  [31:0] rx_freq;
    reg signed [15:0]tx_real,tx_imag;
	 reg [31:0] tx_freq;
	 reg [7:0] att;
	 reg ptt;
	 
	 //*************************************************************************
	 //                  PLLs
	 //*************************************************************************
	
	pll_rx prx (clock_61M, clock_122M, pwm_CLK,clock_100k);
		
	always @(posedge clock_61M)
	begin
		reg_adc_data <=  adc_data;
	end

//RF BPF
filter filter_inst(
		.clock(pwm_CLK), 
		.frequency(rx_freq), 
		.selected_filter(filter));

	
// Receiver
assign test = decim;
	Receiver rx (
		.clock(clock_61M), 
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
		
	reg [63:0] v_control;
	always @(posedge _spi_done)
	
	//convert litle endian to big endian for data from esp32 only
   v_control = {_control[7:0],_control[15:8], _control[23:16],_control[31:24],
				 _control[39:32],_control[47:40],_control[55:48],_control[63:56]};
	
	always @(posedge _spi_done)
		if(n_ptt)
			begin
				ptt <= v_control[63];
				dac_level <= v_control[52:45];
				att = v_control[44:40];
				s_rate <= v_control[39:32];
				rx_freq <= v_control[31:0];
				tx_freq <= 32'b0;
			end
		else
			begin
				ptt <= v_control[63];
				tx_real <= v_control[31:16];
				tx_imag <= v_control[15:0];
				tx_freq <= {v_control[63:32]};
			  end

   //  Transmitter
	Transmitter tx (
	.clock(clock_122M), 
	.reset(!reset),
	.dac_data(dac_data),
	.tx_freq(tx_freq),
	.tx_real(tx_real),
	.tx_imag(tx_imag),
	.cw(n_cw_key),
	.pro_clock(clock_100k)
	);

	wire _spi_done;
	cdc_sync #(1) done(.siga(spi_done), .rstb(1'b0), .clkb(clock_61M), .sigb(_spi_done));
	wire [63:0] _control;
	cdc_sync #(64) cont(.siga(control), .rstb(1'b0), .clkb(clock_61M), .sigb(_control));

   //биты управления и квадратуры приемника из FPGA в MCU
	always @(posedge decim) 
		if(n_ptt)
		 rxToFifo <= {~n_ptt,~n_tune,~n_cw_key,~n_buttons_l,rx_real,~n_buttons_r,rx_imag};
		else
		 rxToFifo <= {~n_ptt,~n_tune,~n_cw_key,~n_buttons_l,8'b0,tx_real,~n_buttons_r,8'b0,tx_imag};
	
   // SPI module, slave, mode3
	spi_slave  #(.WIDTH(64)) spi(
		.rstb(reset),
		.ten(reset),
		//convert big endian to little endian for esp32 only
		.tdata({rxFromFifo[7:0],rxFromFifo[15:8], rxFromFifo[23:16],rxFromFifo[31:24],
				 rxFromFifo[39:32],rxFromFifo[47:40],rxFromFifo[55:48],rxFromFifo[63:56]}),
		.mlb(1'b1),
		.ss(SPI_SS),                   //input
		.sck(SPI_SCK),                 //input
		.sdin(SPI_MOSI),               //input
		.sdout(SPI_MISO),              //output
		.done(spi_done),               //output
		.rdata(control)                //output
	);
wire [10:0] use_fifo;
	fifo fm(
		.aclr(!reset),
		.data(rxToFifo),
		.rdclk(_spi_done),
		.rdreq(1'b1),
		.wrclk(decim),
		.wrreq(reset),
		.q(rxFromFifo)
	);
	
	// Power level
	reg [7:0] dac_level;
	reg [7:0] pwm_cnt;
   always @(posedge pwm_CLK)	
	begin
	   //if (pwm_cnt >= dac_level) level_pwm <= 0;
		//else level_pwm <= 1;
		//pwm_cnt <= pwm_cnt + 1'd1;
		level_pwm <=1;
	end
/*
	reg [26:0] count_l;
	always @(posedge pwm_CLK)
	begin
		count_l <= count_l+27'b1;
	end
	assign led = count_l[22];
*/	
endmodule
