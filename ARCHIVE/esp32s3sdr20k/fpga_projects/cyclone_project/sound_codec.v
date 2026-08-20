//////////////////////////////////////////////////////////////////////////////////
// Module for PCM1771 codec
// 
// Mode Master I2S 
// 
// Autor David Fainitski
// 06.02.15   
//
//////////////////////////////////////////////////////////////////////////////////

module sound_codec(
	input SCKI,       //12.288 MHz
	input eth_nRESET,
	input signed [31:0] cdc_sample,		// next sample to codec
	output reg get_next_dac,
	output reg LRCLK,
	output reg DATA,
	input BCK,       //16*2*48kHz, 1.536 MHz
	input tone_clock,
	input pro_clock, // 200kHz
   input cw_on,
	input cw_on_f,
	input key_input,
	input [7:0] st_volume_sw,
	input key_down,
	input tx_en
	);
	
	
	// Tone generating
	reg [9:0] tone_count = 0;
	wire signed [15:0] sine_tone;
	sine_table si_tone (.address(tone_count), .clock(tone_clock), .q(sine_tone));
	always @(negedge tone_clock) 
	  begin
		if (tone_count != 999) tone_count <= tone_count + 1;
		else tone_count <= 0;
	  end
	
	// Sound profiling
	reg [9:0] pro_count = 0;

	always @(posedge pro_clock)
	 begin
	    if ( (cw_on && ~key_input)  || sound_on )
		     begin
			    if (pro_count != 511) pro_count <= pro_count + 1;
			  end
		 else
		     begin
			    if (pro_count != 0)   pro_count <= pro_count - 1;
			  end
	 end
	 
	 wire [15:0] s_profile;
	  profile sine_profile (.address(pro_count), .clock(pro_clock), .q(s_profile));
	 
	 wire signed [15:0] pro_sound;
	 wire signed [15:0] sound_cdc;
	 wire signed [15:0] source = cw_on_f ? sine_tone : cdc_sample[15:0] ;
	 
	 mult_16x16 tone_pro (source, s_profile, pro_sound);
	 
	 // Side Tone Volume
	reg [7:0] tone_vol = 0; 
	reg volume_state = 0;
   always @(posedge pro_clock)
   begin
	   if (~tx_en) volume_state <= 0;
		else
		begin
        case (volume_state)
	       0: begin if (st_volume_sw != 0 && key_down) volume_state <= 1; else tone_vol <= 8'd128;  end
		  	 1: tone_vol <= st_volume_sw ;  
	      endcase	
		 end
   end	
	
	 wire [15:0] volume = cw_on_f  ? {1'b0,tone_vol[7:0], 7'b0} : 16'd32767;
	 mult_16x16 tone_volume (pro_sound, volume, sound_cdc);
	
   // Sound delay 100msec
	reg sound_on ;
	parameter sound_delay = 110; // Delay in msec , 320msec max
	reg [15:0] sd_timer = 0;
   always @ (posedge pro_clock)	// 200 kHz
	begin
	   if ( key_down ) begin sound_on <= 0; sd_timer <= sound_delay * 200 ; end
	   else 
		begin   
          if (sd_timer != 0) sd_timer <= sd_timer - 1;
			 else sound_on = 1;
		end
	end

	// Send to CODEC
	reg [3:0] bit_cnt = 0;
	reg signed [31:0]i2s_sample;
	//
   always @(negedge BCK)
   begin
       if (eth_nRESET == 0) bit_cnt <= 0;
       else
	      begin
            //setting D_OUT
	         if (LRCLK) DATA <= i2s_sample [16 + bit_cnt];// 
	         else       DATA <= i2s_sample [bit_cnt];// 
            //Forming LR_CLK
            if (bit_cnt == 0) LRCLK <= ~LRCLK;  
	         //
	         if (bit_cnt == 0 && LRCLK == 0) begin get_next_dac <= ~ get_next_dac;  // request next cdc_sample	
	                                         i2s_sample <= {sound_cdc[15:0], sound_cdc[15:0]}; 	end		
            //
	         bit_cnt <= bit_cnt - 1;
	     end 
   end
	
	
endmodule