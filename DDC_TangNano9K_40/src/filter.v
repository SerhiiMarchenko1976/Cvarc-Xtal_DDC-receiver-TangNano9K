module filter(clock, frequency, selected_filter);
input  wire        clock;
input  wire [31:0] frequency;	// Принимает 32-битный фазовый шаг гетеродина
output reg   [4:0] selected_filter;
		
always @(posedge clock)  
begin 
	// Константы пересчитаны строго под фазовое слово CORDIC для частоты 63.00 МГц!
	if      (frequency > 32'd1090785345) selected_filter <= 5'b00001; // > 16.0 MHz
	else if (frequency > 32'd545392672)  selected_filter <= 5'b00010; // > 8.0 MHz
	else if (frequency > 32'd272696336)  selected_filter <= 5'b00100; // > 4.0 MHz
	else if (frequency > 32'd136348168)  selected_filter <= 5'b01000; // > 2.0 MHz
	else 	selected_filter <= 5'b10000 ;                             // < 2.0 MHz
end 
endmodule
