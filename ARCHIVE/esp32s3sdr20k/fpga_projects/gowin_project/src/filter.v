module filter(clock, frequency, selected_filter);
input  wire        clock;
input  wire [31:0] frequency;	
output reg   [4:0] selected_filter;
		
always @(posedge clock)  
begin 
	if      (frequency > 31'd16000000) selected_filter <= 5'b00001;
	else if (frequency > 31'd8000000)  selected_filter <= 5'b00010;
	else if (frequency > 31'd4000000)  selected_filter <= 5'b00100;
	else if (frequency > 31'd2000000)  selected_filter <= 5'b01000;
	else 	selected_filter <= 5'b10000 ;
end 
endmodule