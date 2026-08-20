
//////////////////////////////////////////////////////////////////////////////////

module Transmitter(
	input clock,
	input reset,
	output signed [13:0] dac_data,
	input [31:0] tx_freq,
   input signed [15:0] tx_real,
	input signed [15:0] tx_imag,
	input cw,
	input pro_clock
	);
wire signed [15:0] _tx_real;
cdc_sync #(16)
	re_data (.siga(tx_real), .rstb(1'b0), .clkb(clock), .sigb(_tx_real));
	
wire signed [15:0] _tx_imag;
cdc_sync #(16)
	im_data (.siga(tx_imag), .rstb(1'b0), .clkb(clock), .sigb(_tx_imag));
	
wire _cw;
cdc_sync #(1)
	c_w (.siga(cw), .rstb(1'b0), .clkb(clock), .sigb(_cw));
	
wire _reset;
cdc_sync #(1)
	rst_t (.siga(reset), .rstb(1'b0), .clkb(clock), .sigb(_reset));
	
wire _pro_clock;
cdc_sync #(1)
	p_clk (.siga(pro_clock), .rstb(1'b0), .clkb(clock), .sigb(_pro_clock));
	reg  signed [15:0] tx_reg_real, tx_reg_imag;
	always @(posedge req1) begin tx_reg_real <= _tx_real; tx_reg_imag <= _tx_imag;  end 
	
   // TX phase count
	//localparam M2 = 32'd1172812403;  // B57 = 2^57.   M2 = B57/122880000
   //localparam M3 = 32'd16777216;   // M3 = 2^24, used to round the result
	//wire [63:0] ratio = tx_freq * M2 + M3;
   wire [31:0] tx_tune_phase = tx_freq;//ratio[56:25]; // 32'd251658240; // 7 MHz   
	
	
	// Interpolate I/Q samples in memory from 48 kHz to the clock frequency
	wire req1, req2;
	wire signed [19:0] y1_r, y1_i, y2_r, y2_i;
	FirInterp8 fi (clock, req2, req1, tx_reg_real, tx_reg_imag, y1_r, y1_i);
	CicInterpM5 #(.RRRR(320), .IBITS(20), .OBITS(20), .GBITS(34))
        	in2 (clock, 1'd1, req2, y1_r, y1_i, y2_r, y2_i);

//CW profile memory
wire [15:0] cw_profile;
reg [8:0] pro_cnt;
profile cw_pro (.address(pro_cnt), .clock(_pro_clock), .q(cw_profile));
always @ (posedge _pro_clock)
   //if(!_reset) 
	pro_cnt <= 1'd0;
	//else if(_cw & pro_cnt!=511) pro_cnt <= pro_cnt + 1'd1;
	//else if(!_cw & pro_cnt>0) pro_cnt <= pro_cnt - 1'd1;
		
// CW profiling
wire [15:0] cw_level;	
parameter MAX_CW_LEVEL = 16'd39000;	// 65535 max	
mult_16x16_uns cw_mult (MAX_CW_LEVEL, cw_profile, cw_level);
	
// Switch sample source	
wire [15:0] ci_real =  pro_cnt!=0 ? cw_level : y2_r[19:4];  
wire [15:0] ci_imag =  pro_cnt!=0 ? 16'd0    : y2_i[19:4];

	// Tune transmitter with CORDIC
   wire signed [15:0]cordic_out_i, cordic_out_q;	
	cordic_tx #(.OUT_WIDTH(16))
 		cordic_inst (.clock(clock), .frequency(tx_tune_phase), .in_data_I(ci_real),			
		.in_data_Q(ci_imag), .out_data_I(cordic_out_i), .out_data_Q(cordic_out_q));

		
	assign dac_data = {cordic_out_i[15], cordic_out_i[14:2]};
	
endmodule 