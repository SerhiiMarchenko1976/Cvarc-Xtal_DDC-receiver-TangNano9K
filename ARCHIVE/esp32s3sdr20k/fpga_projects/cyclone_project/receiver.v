

module Receiver(
	input clock,
	input reset,
	input [11:0] adc_data,
	input [31:0] rx_freq,
   output reg signed [23:0] rx_real,
	output reg signed [23:0] rx_imag,
	input [7:0] s_rate,
	output decim
	);
	
	
	always @(posedge decim) begin  rx_real <= decim_real; rx_imag <= decim_imag;  end
//always @(posedge cic_outstrobe_2) begin rx_real <=cic_outdata_I2;rx_imag <=cic_outdata_Q2;end

	// RX phase count
	//localparam M2 = 32'd2345624805;  // B57 = 2^57.   M2 = B57/61440000
   //localparam M3 = 32'd16777216;   // M3 = 2^24, used to round the result
	//wire [63:0] ratio = rx_freq * M2 + M3;
   wire [31:0] rx_tune_phase = rx_freq;//ratio[56:25];
	

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
 

wire signed [21:0] cordic_outdata_I;
wire signed [21:0] cordic_outdata_Q;
  

//------------------------------------------------------------------------------
//         register-based CIC decimator #1, decimation factor 128/64/32
//------------------------------------------------------------------------------
//I channel
varcic #(.STAGES(4), .DECIMATION(32), .IN_WIDTH(22), .ACC_WIDTH(56), .OUT_WIDTH(24))
  varcic_inst_I1(
    .clock(clock),
    .in_strobe(1'b1),
    .extra_decimation (s_rate[1:0]),  //00=24, 01=48, 10=96 kHz
    .out_strobe(cic_outstrobe_1),
    .in_data(cordic_outdata_I),
    .out_data(cic_outdata_I1)
    );


//Q channel
varcic #(.STAGES(4), .DECIMATION(32), .IN_WIDTH(22), .ACC_WIDTH(56), .OUT_WIDTH(24))
  varcic_inst_Q1(
    .clock(clock),
    .in_strobe(1'b1),
    .extra_decimation(s_rate[1:0]),  //00=24, 01=48, 10=96 kHz
    .out_strobe(),
    .in_data(cordic_outdata_Q),
    .out_data(cic_outdata_Q1)
    );


wire cic_outstrobe_1;
wire signed [23:0] cic_outdata_I1;
wire signed [23:0] cic_outdata_Q1;

//------------------------------------------------------------------------------
//            memory-based CIC decimator #2, decimation factor 10
//------------------------------------------------------------------------------
//I channel
memcic #(.STAGES(13), .DECIMATION(10), .ACC_WIDTH(70)) 
  memcic_inst_I(
    .clock(clock),
    .in_strobe(cic_outstrobe_1),
    .out_strobe(cic_outstrobe_2),
    .in_data(cic_outdata_I1),
    .out_data(cic_outdata_I2)
    );
//Q channel
memcic #(.STAGES(13), .DECIMATION(10), .ACC_WIDTH(70)) 
  memcic_inst_Q(
    .clock(clock),
    .in_strobe(cic_outstrobe_1),
    .out_strobe(),
    .in_data(cic_outdata_Q1),
    .out_data(cic_outdata_Q2)
    );

	wire cic_outstrobe_2;
	wire signed [23:0] cic_outdata_I2;
	wire signed [23:0] cic_outdata_Q2;		 

	  
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


//__________________________________________________________________________________________________________

//Modified to correct work with decimation rate 10.

//__________________________________________________________________________________________________________


module varcic1 (clock, in_strobe,  out_strobe, in_data, out_data );

  //design parameters
  parameter [6:0] decimation = 10;
  parameter STAGES = 5; //  Sections of both Comb and Integrate
  parameter [5:0] IN_WIDTH = 18;
  parameter OUT_WIDTH = 18;

  // calculated parameters
  // ACC_WIDTH = IN_WIDTH + Ceil(STAGES * log2(MAX_DECIMATION));
  parameter ACC_WIDTH = 34;
  
  input clock;
  input in_strobe;
  output reg out_strobe;

  input signed [IN_WIDTH-1:0] in_data;
  output signed[OUT_WIDTH-1:0] out_data;


//------------------------------------------------------------------------------
//                               control
//------------------------------------------------------------------------------
reg [7:0] sample_no = 0;

generate
	always @(posedge clock)
		if (in_strobe) 
			if (sample_no == (decimation - 1'd1)) begin
				sample_no <= 0;
				out_strobe <= 1;
			end else begin
				sample_no <= sample_no + 1'd1;
     				out_strobe <= 0;
			end
		else
			out_strobe <= 0;
endgenerate

//------------------------------------------------------------------------------
//                                stages
//------------------------------------------------------------------------------
reg signed [ACC_WIDTH-1:0] integrator_data [1:STAGES];
reg signed [ACC_WIDTH-1:0] comb_data [1:STAGES];
reg signed [ACC_WIDTH-1:0] comb_last [0:STAGES];

always @(posedge clock) begin
	integer index;

	//  Integrators
	if(in_strobe) begin
		integrator_data[1] <= integrator_data[1] + in_data;
		for(index = 1; index < STAGES; index = index + 1) begin
			integrator_data[index + 1] <= integrator_data[index] + integrator_data[index+1];
		end
	end

	// Combs
	if(out_strobe) begin
		comb_data[1] <= integrator_data[STAGES] - comb_last[0];
		comb_last[0] <= integrator_data[STAGES];
		for(index = 1; index < STAGES; index = index + 1) begin
			comb_data[index + 1] <= comb_data[index] - comb_last[index];
			comb_last[index] <= comb_data[index]; 
		end
	end
end

//------------------------------------------------------------------------------
//                            output rounding
//------------------------------------------------------------------------------

assign out_data = comb_data[STAGES][(ACC_WIDTH-1) -: OUT_WIDTH] + comb_data[STAGES][(ACC_WIDTH-1) - OUT_WIDTH - 1];


endmodule
