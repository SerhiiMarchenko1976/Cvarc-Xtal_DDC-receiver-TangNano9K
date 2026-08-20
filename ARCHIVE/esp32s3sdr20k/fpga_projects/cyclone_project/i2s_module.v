//  Module for rx9cim, 32 bit I2S master
module i2s_module (
   input reset,
   input MCLK,
   output BCLK,
   output reg LRCLK,
   input	DIN,
	output reg DOUT,
   input	[23:0] rx_real,
   input	[23:0] rx_imag,
   output reg [15:0] tx_real,
	output reg [15:0] tx_imag,
	input [7:0] s_rate
	);

	// BCLK generate
	assign BCLK = s_rate[1:0] == 0 ? b_cnt[2] : (s_rate[1:0] == 1 ? b_cnt[1] : (s_rate[1:0] == 2 ? b_cnt[0] : b_cnt[0]));
	reg [2:0] b_cnt = 0;
	always @(posedge MCLK) b_cnt <= b_cnt + 1'd1;
	
	//
   reg [5:0] bit_cnt;
   reg [63:0] buffer_rx, buffer_tx;
   always @(negedge BCLK)
   begin
      if (reset == 0) bit_cnt <= 0;
      else
	   begin
	      bit_cnt = bit_cnt - 1'd1;
		   if (bit_cnt == 31)
		   begin   //write sample to buffer
		      buffer_rx = {rx_real, 8'b0, rx_imag, 8'b0}; 
			   tx_real = buffer_tx[63:48];
			   tx_imag = buffer_tx[31:16];
		   end 
         DOUT = buffer_rx[bit_cnt];  //setting D_OUT
         if (bit_cnt == 0) LRCLK = 1; if (bit_cnt == 32) LRCLK = 0; //Forming LR_CLK
	      //
	   end 
   end
   always @(posedge BCLK) //Recieve TX samples
   begin
      buffer_tx[bit_cnt] = DIN;
   end

endmodule
	
