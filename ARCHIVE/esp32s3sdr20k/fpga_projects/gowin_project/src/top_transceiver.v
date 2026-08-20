

module top_trx(
	input clk61,			//61.440MHz clock from ms5351 channel 0 on board tang nano 20k
    input clk96,            //96MHz     clock from ms5351 channel 1 on board tang nano 20k
	input start,

    // DAC_ADC interface
    inout [13:0] data_io,
    output adc_clk,
    output dac_clk,
    output adc_off,        //to pin OEB AD9226
    output reg dac_pwm,

	// SPI bus
	input  SPI_MOSI,
	output SPI_MISO,
	input  SPI_SS,
	input  SPI_SCK,

	//control
	input n_ptt,              //здесь все входы строго с сильной подтяжкой к +3,3v
	input n_tune,
	input n_cw_key,
    output reg preamp_on,
	output reg att_on,
	output [4:0]filter,      // b10000-1..2MHz,
							 // b01000-2..4MHz,
							 // b00100-4..8MHz,
							 // b00010-8..16MHz,
							 // b00001-16..30MHz
	output test
);

wire [13:0] dac_data;

assign data_io = n_ptt ? 14'bZ:dac_data;
assign adc_off = ~n_ptt;
wire reset;
assign adc_clk = n_ptt ? clk61 : 1'b0;
assign dac_clk = n_ptt ? 1'b0  : clk96;
assign reset = start;
wire decim;
reg [1:0] s_rate;
wire signed [23:0] rx_real, rx_imag;
wire [63:0] control;
wire [63:0] rxFromFifo;
reg  [63:0] rxToFifo;
reg  [11:0] reg_adc_data;
wire spi_done;
reg  [31:0] rx_freq;
reg [31:0] tx_freq;
reg signed [15:0]tx_real,tx_imag;

always @(posedge clk61 && n_ptt)
	begin
		reg_adc_data <= data_io[11:0];
	end

    Gowin_rPLL pll(
        .clkout(clk10M), //output clkout
        .clkoutd(clk100K), //output clkoutd
        .clkin(clk61) //input clkin
    );

//RF BPF
filter filter_inst(
		.clock(clk10M), 
		.frequency(rx_freq), 
		.selected_filter(filter));

// Receiver
assign test = decim;
	Receiver rx (
		.clock(clk61), 
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

	wire _spi_done;
	cdc_sync #(1) done(.siga(spi_done), .rstb(1'b0), .clkb(clk61), .sigb(_spi_done));
	wire [63:0] _control;
	cdc_sync #(64) cont(.siga(control), .rstb(1'b0), .clkb(clk61), .sigb(_control));
		
	reg [63:0] v_control;

	always @(posedge _spi_done)  begin
        //convert litle endian to big endian for data from esp32 only
        v_control = {_control[7:0],_control[15:8], _control[23:16],_control[31:24],
				 _control[39:32],_control[47:40],_control[55:48],_control[63:56]};
		if(n_ptt)
			begin
				dac_level <= v_control[43:36];
				att_on <= v_control[35];
                preamp_on = v_control[34];
				s_rate <= v_control[33:32];
				rx_freq <= v_control[31:0];
                tx_freq <= 32'b0;
			end
		else
			begin
			  tx_real <= v_control[31:16];
			  tx_imag <= v_control[15:0];
			  tx_freq <= v_control[63:32];
			  end
    end
   //  Transmitter
	Transmitter tx (
	.clock(clk96), 
	.reset(!reset),
	.dac_data(dac_data),
	.tx_freq(tx_freq),
	.tx_real(tx_real),
	.tx_imag(tx_imag)
	);

   //биты управления и квадратуры приемника из FPGA в MCU
	always @(decim) rxToFifo <= {~n_ptt,~n_tune,~n_cw_key,5'b0,rx_real,8'b0,rx_imag};

   // SPI module, slave, mode3
	spi_slave  #(.WIDTH(64)) spi(
		.rstb(reset),              //input
		.ten(reset),
		//convert big endian to little endian for esp32 only
		.tdata({rxFromFifo[7:0],rxFromFifo[15:8], rxFromFifo[23:16],rxFromFifo[31:24],
				 rxFromFifo[39:32],rxFromFifo[47:40],rxFromFifo[55:48],rxFromFifo[63:56]}),
		.mlb(1'b1),
		.ss(SPI_SS),                   //input
		.sck(SPI_SCK),                //input
		.sdin(SPI_MOSI),               //input
		.sdout(SPI_MISO),              //output
		.done(spi_done),               //output
		.rdata(control)                //output
	);

	fifo fifo_main(
		.Data(rxToFifo), //input [63:0] Data
		.WrReset(!reset), //input WrReset
		.RdReset(!reset), //input RdReset
		.WrClk(decim), //input WrClk
		.RdClk(_spi_done), //input RdClk
		.WrEn(reset), //input WrEn
		.RdEn(1'b1), //input RdEn
		.Q(rxFromFifo), //output [63:0] Q
		.Empty(), //output Empty
		.Full() //output Full
	);
	
  // Power level
  reg [7:0] dac_level;
  reg [7:0] pwm_cnt;
  always @(posedge clk10M)	
	begin
	   if (pwm_cnt >= dac_level) dac_pwm <= 0;
		else dac_pwm <= 1;
		pwm_cnt <= pwm_cnt + 1'd1;
  end

endmodule