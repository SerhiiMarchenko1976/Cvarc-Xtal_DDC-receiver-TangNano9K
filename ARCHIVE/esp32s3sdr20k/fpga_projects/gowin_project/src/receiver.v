

module Receiver(
	input clock,
	input reset,
	input [11:0] adc_data,
	input [31:0] rx_freq,
   output reg signed [23:0] rx_real,
	output reg signed [23:0] rx_imag,
	input [1:0] s_rate,
	output decim
	);
	
	
	always @(posedge decim) begin  rx_real <= decim_real; rx_imag <= decim_imag;  end
//always @(posedge cic_outstrobe_2) begin rx_real <=cic_outdata_I2;rx_imag <=cic_outdata_Q2;end

	// RX phase count
	localparam M2 = 32'd2345624805;  // B57 = 2^57.   M2 = B57/61440000
   localparam M3 = 32'd16777216;   // M3 = 2^24, used to round the result
	wire [63:0] ratio = rx_freq * M2 + M3;
   wire [31:0] rx_tune_phase = ratio[56:25];
	

wire signed [21:0] cordic_outdata_I;
wire signed [21:0] cordic_outdata_Q;

wire cic_outstrobe_1;
wire signed [23:0] cic_outdata_I1;
wire signed [23:0] cic_outdata_Q1;

wire cic_outstrobe_2;
wire signed [23:0] cic_outdata_I2;
wire signed [23:0] cic_outdata_Q2;		 

//------------------------------------------------------------------------------
//                               cordic
//------------------------------------------------------------------------------
cordic_rx cordic_rx(
  .clock(clock),
  .in_data({adc_data, 4'b0}),  //16 bit input
  .frequency(rx_tune_phase),         //32 bit
  .out_data_I(cordic_outdata_I), //22 bit
  .out_data_Q(cordic_outdata_Q)
  );
 

  

//------------------------------------------------------------------------------
//         register-based CIC decimator #1, decimation factor 32/64/128
//------------------------------------------------------------------------------
//I channel
varcic #(.STAGES(4), .DECIMATION(32), .IN_WIDTH(22), .ACC_WIDTH(50), .OUT_WIDTH(24))
  varcic_inst_I1(
    .clock(clock),
    .in_strobe(1'b1),
    .extra_decimation (s_rate[1:0]),  //00=24, 01=48, 10=96 kHz
    .out_strobe(cic_outstrobe_1),
    .in_data(cordic_outdata_I),
    .out_data(cic_outdata_I1)
    );


//Q channel
varcic #(.STAGES(4), .DECIMATION(32), .IN_WIDTH(22), .ACC_WIDTH(50), .OUT_WIDTH(24))
  varcic_inst_Q1(
    .clock(clock),
    .in_strobe(1'b1),
    .extra_decimation(s_rate[1:0]),  //00=24, 01=48, 10=96 kHz
    .out_strobe(),
    .in_data(cordic_outdata_Q),
    .out_data(cic_outdata_Q1)
    );


//------------------------------------------------------------------------------
//            memory-based CIC decimator #2, decimation factor 10
//------------------------------------------------------------------------------

//I channel
cic #(.STAGES(6), .DECIMATION(10), .IN_WIDTH(24), .ACC_WIDTH(60), .OUT_WIDTH(24))
  cic_inst_I1(
    .clock(clock),
    .in_strobe(1'b1),
    .out_strobe(cic_outstrobe_2),
    .in_data(cic_outdata_I1),
    .out_data(cic_outdata_I2)
    );


//Q channel
cic #(.STAGES(6), .DECIMATION(10), .IN_WIDTH(24), .ACC_WIDTH(60), .OUT_WIDTH(24))
  cic_inst_IQ(
    .clock(clock),
    .in_strobe(1'b1),
    .out_strobe(),
    .in_data(cic_outdata_Q1),
    .out_data(cic_outdata_Q2)
    );

	  
//------------------------------------------------------------------------------
//                     FIR coefficients and sequencing
//------------------------------------------------------------------------------
wire signed [23:0] fir_coeff;

fir_coeffs fir_coeffs_inst(
  .clock(clock),
  .start(cic_outstrobe_2),
  .coeff(fir_coeff)
  );
 
//------------------------------------------------------------------------------
//                            FIR decimator/2
//------------------------------------------------------------------------------
wire [23:0] decim_real, decim_imag;

fir #(.OUT_WIDTH(24))
  fir_inst_I(
    .clock(clock),
    .start(cic_outstrobe_2), 
    .coeff(fir_coeff),
    .in_data(cic_outdata_I2),
    .out_data(decim_real),
    .out_strobe(decim)
    );


fir #(.OUT_WIDTH(24))
  fir_inst_Q(
    .clock(clock),
    .start(cic_outstrobe_2),
    .coeff(fir_coeff),
    .in_data(cic_outdata_Q2),
    .out_data(decim_imag),
    .out_strobe()
    );

	
endmodule	