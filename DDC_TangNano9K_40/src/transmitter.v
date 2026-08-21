
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

module Transmitter(
	input clock,
	input reset,
	output signed [13:0] dac_data,
	input [31:0] tx_freq,
	input signed [15:0] tx_real,
	input signed [15:0] tx_imag	
	);
	
	reg  signed [15:0] tx_reg_real, tx_reg_imag;

	always @(posedge clock) begin
		if (req1) begin 
			tx_reg_real <= tx_real; 
			tx_reg_imag <= tx_imag;  
		end 
	end

	// Частота напрямую без тяжелой математики
	wire [31:0] tx_tune_phase = tx_freq; 
	
	// Интерполятор I/Q
	wire req1, req2;
	wire signed [19:0] y1_r, y1_i;
	
	// Новые шины уменьшенной разрядности для выхода CIC
	wire signed [14:0] y2_r, y2_i; 
	
	FirInterp8 fi (clock, req2, req1, tx_reg_real, tx_reg_imag, y1_r, y1_i);

	// ОПТИМИЗАЦИЯ: Снижаем IBITS/OBITS до 16, а GBITS до 28. 
	// На вход CIC подаем старшие 16 бит из 20-битного выхода FIR (y1_r[19:4]).
	// Это уберет около 400-600 ALU из логики!
	//CicInterpM5 #(.RRRR(250), .IBITS(16), .OBITS(16), .GBITS(28))
    //    in2 (clock, 1'd1, req2, y1_r, y1_i, y2_r, y2_i);
	// ОПТИМИЗИРОВАНО: Снижаем разрядность тракта передачи на 1 бит для прохождения лимита 8640
	
	// БЫЛО: CicInterpM5 #(.RRRR(250), ... )
	// СТАЛО: Шаг интерполяции 156 идеально адаптирует передатчик под частоту 60 МГц!
	CicInterpM5 #(.RRRR(156), .IBITS(15), .OBITS(15), .GBITS(24))
        in2 (clock, 1'd1, req2, y1_r[19:5], y1_i[19:5], y2_r, y2_i);




    //CicInterpM5 #(.RRRR(250), .IBITS(15), .OBITS(15), .GBITS(27)) // Уменьшили GBITS до 27, IBITS/OBITS до 15
        
    //in2 (clock, 1'd1, req2, y1_r, y1_i, y2_r, y2_i);
    /*
    in2 (
			.clock(clock), 
			.enable(1'd1), 
			.req(req2), 
			.in_R(y1_r[19:5]), // Берём 15 бит вместо 16
			.in_I(y1_i[19:5]), 
			.out_R(y2_r), 
			.out_I(y2_i)
		);
*/
       

	// Передатчик CORDIC (теперь принимает шины целиком, так как они уже 16-битные)
	wire signed [13:0] cordic_out_i;	
		//------------------------------------------------------------------------------
	// ИСПРАВЛЕНО: Синтаксически корректное расширение знака с 15 до 16 бит
	// Конструкция {y2_i[14], y2_i} берёт старший знаковый бит и дописывает к числу
	//------------------------------------------------------------------------------
	cordic_tx #(.OUT_WIDTH(14))
 		cordic_inst (
			.clock(clock), 
			.frequency(tx_tune_phase), 
			.in_data_I({y2_i[14], y2_i}), // Берем 14-й (знаковый) бит и приписываем к шине
			.in_data_Q({y2_r[14], y2_r}), // Берем 14-й (знаковый) бит и приписываем к шине
			.out_data_I(cordic_out_i), 
			.out_data_Q()
		);

	
	//assign dac_data = {cordic_out_i[15], cordic_out_i[14:2]};
    // ИСПРАВЛЕНО ДЛЯ ОШИБКИ EX3784: Прямая выдача 14-битного CORDIC на ЦАП.
	// Больше никаких индексов [15:1] или [14:1], которые вылетали за пределы шины!
	assign dac_data = cordic_out_i;
	
endmodule
