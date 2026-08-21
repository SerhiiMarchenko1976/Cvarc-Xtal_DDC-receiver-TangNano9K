module Receiver(
	input clock,
	input reset,
	input [11:0] adc_data,
	input [31:0] rx_freq,
	output reg signed [15:0] rx_real,
	output reg signed [15:0] rx_imag,
	input [1:0] s_rate,
	output decim
	);
	
	always @(posedge clock) begin
		rx_real <= decim_real[23:8]; 
		rx_imag <= decim_imag[23:8];
	end

	wire [31:0] rx_tune_phase = rx_freq;
	
	wire signed [21:0] cordic_outdata_I;
	wire signed [21:0] cordic_outdata_Q;

	wire cic_outstrobe_1;
	wire signed [19:0] cic_outdata_I1;
	wire signed [19:0] cic_outdata_Q1;

	wire cic_outstrobe_2;
	wire signed [23:0] cic_outdata_I2;
	wire signed [23:0] cic_outdata_Q2;

//------------------------------------------------------------------------------
//                               cordic
//------------------------------------------------------------------------------
cordic_rx cordic_rx(
  .clock(clock),
  .in_data({adc_data, 4'b0}),    
  .frequency(rx_tune_phase),     
  .out_data_I(cordic_outdata_I), 
  .out_data_Q(cordic_outdata_Q)
  );

//------------------------------------------------------------------------------
//   Shared VARCIC decimator #1 (Оптимизирована ширина аккумулятора до 32 бит!)
//------------------------------------------------------------------------------
reg varcic_mux_sel = 0;
wire signed [19:0] varcic_mux_in; 
wire signed [19:0] varcic_shared_out; 
wire varcic_shared_strobe;

always @(posedge clock) begin
    varcic_mux_sel <= ~varcic_mux_sel;
end

assign varcic_mux_in = varcic_mux_sel ? cordic_outdata_I[21:2] : cordic_outdata_Q[21:2];

// ИЗМЕНЕНО: .ACC_WIDTH(32) вместо 35. Это убирает сотни параллельных дорожек
varcic #(.STAGES(4), .DECIMATION(32), .IN_WIDTH(20), .ACC_WIDTH(32), .OUT_WIDTH(20))
  varcic_inst_SHARED(
    .clock(clock),
    .in_strobe(1'b1),
    .extra_decimation(s_rate[1:0]),
    .out_strobe(varcic_shared_strobe),
    .in_data(varcic_mux_in),
    .out_data(varcic_shared_out)
    );

reg signed [19:0] cic_outdata_I1_reg = 0;
reg signed [19:0] cic_outdata_Q1_reg = 0;
reg cic_outstrobe_1_reg = 0;

always @(posedge clock) begin
    if (varcic_shared_strobe) begin
        if (varcic_mux_sel) begin
            cic_outdata_I1_reg <= varcic_shared_out;
            cic_outstrobe_1_reg <= 1'b1; 
        end else begin
            cic_outdata_Q1_reg <= varcic_shared_out;
            cic_outstrobe_1_reg <= 1'b0;
        end
    end else begin
        cic_outstrobe_1_reg <= 1'b0;
    end
end

assign cic_outdata_I1 = cic_outdata_I1_reg;
assign cic_outdata_Q1 = cic_outdata_Q1_reg;
assign cic_outstrobe_1 = cic_outstrobe_1_reg;

//------------------------------------------------------------------------------
//   Shared CIC decimator #2 (Оптимизирована ширина аккумулятора до 30 бит!)
//------------------------------------------------------------------------------
reg cic2_mux_sel = 0;
wire signed [19:0] cic2_mux_in;
wire signed [23:0] cic2_shared_out;
wire cic2_shared_strobe;

always @(posedge clock) begin
    cic2_mux_sel <= ~cic2_mux_sel;
end

assign cic2_mux_in = cic2_mux_sel ? cic_outdata_I1 : cic_outdata_Q1;

// ИЗМЕНЕНО: .ACC_WIDTH(30) вместо 38. Освобождает критический ресурс трассировки ПЛИС
cic #(.STAGES(4), .DECIMATION(10), .IN_WIDTH(20), .ACC_WIDTH(30), .OUT_WIDTH(24))
  cic_inst_SHARED(
    .clock(clock),
    .in_strobe(cic_outstrobe_1), 
    .out_strobe(cic2_shared_strobe),
    .in_data(cic2_mux_in),
    .out_data(cic2_shared_out)
    );

reg signed [23:0] cic_outdata_I2_reg = 0;
reg signed [23:0] cic_outdata_Q2_reg = 0;
reg cic_outstrobe_2_reg = 0;

always @(posedge clock) begin
    if (cic2_shared_strobe) begin
        if (cic2_mux_sel) begin
            cic_outdata_I2_reg <= cic2_shared_out;
            cic_outstrobe_2_reg <= 1'b1; 
        end else begin
            cic_outdata_Q2_reg <= cic2_shared_out;
            cic_outstrobe_2_reg <= 1'b0;
        end
    end else begin
        cic_outstrobe_2_reg <= 1'b0;
    end
end

assign cic_outdata_I2 = cic_outdata_I2_reg;
assign cic_outdata_Q2 = cic_outdata_Q2_reg;
assign cic_outstrobe_2 = cic_outstrobe_2_reg;

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
    .out_strobe(decim),
    .test()
    );

fir #(.OUT_WIDTH(24))
  fir_inst_Q(
    .clock(clock),
    .start(cic_outstrobe_2),
    .coeff(fir_coeff),
    .in_data(cic_outdata_Q2),
    .out_data(decim_imag),
    .out_strobe(),
    .test()
    );
	
endmodule
