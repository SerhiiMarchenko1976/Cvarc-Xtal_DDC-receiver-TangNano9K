module cic( clock, in_strobe, out_strobe, in_data, out_data );

//design parameters
parameter STAGES = 3;
parameter DECIMATION = 16;  
parameter IN_WIDTH = 18;
parameter ACC_WIDTH = 30;
parameter OUT_WIDTH = 24;

input clock;
input in_strobe;
output reg out_strobe;
input signed [IN_WIDTH-1:0] in_data;
output signed [OUT_WIDTH-1:0] out_data;

// Сигнал для чередования каналов внутри CIC
reg channel_state = 0;
always @(posedge clock) begin
    channel_state <= ~channel_state;
end

//------------------------------------------------------------------------------
//                               control
//------------------------------------------------------------------------------
reg [15:0] sample_no;
initial sample_no = 16'd0;

always @(posedge clock)
  if (in_strobe)
    begin
    if (sample_no == (DECIMATION-1))
      begin
      sample_no <= 0;
      out_strobe <= 1;
      end
    else
      begin
      sample_no <= sample_no + 16'd1;
      out_strobe <= 0;
      end
    end
  else
    out_strobe <= 0;

//------------------------------------------------------------------------------
//                                stages
//------------------------------------------------------------------------------
wire signed [ACC_WIDTH-1:0] integrator_data [0:STAGES];
wire signed [ACC_WIDTH-1:0] comb_data [0:STAGES];

assign integrator_data[0] = in_data;
assign comb_data[0] = integrator_data[STAGES];

genvar i;
generate
  for (i=0; i<STAGES; i=i+1)
    begin : cic_stages

    // Интеграторы теперь знают про переключение каналов
    cic_integrator1 #(ACC_WIDTH) cic_integrator_inst(
      .clock(clock),
      .strobe(in_strobe),
      .chan_sel(channel_state),
      .in_data(integrator_data[i]),
      .out_data(integrator_data[i+1])
      );

    // Гребенчатые фильтры теперь знают про переключение каналов
    cic_comb1 #(ACC_WIDTH) cic_comb_inst(
      .clock(clock),
      .strobe(out_strobe),
      .chan_sel(channel_state),
      .in_data(comb_data[i]),
      .out_data(comb_data[i+1])
      );
    end
endgenerate

//------------------------------------------------------------------------------
//                            output rounding
//------------------------------------------------------------------------------
assign out_data = comb_data[STAGES][ACC_WIDTH-1:ACC_WIDTH-OUT_WIDTH] +
  {{(OUT_WIDTH-1){1'b0}}, comb_data[STAGES][ACC_WIDTH-OUT_WIDTH-1]};

endmodule


//------------------------------------------------------------------------------
//    Модуль гребенчатого фильтра с поддержкой разделения каналов
//------------------------------------------------------------------------------
module cic_comb1( clock, strobe, chan_sel, in_data, out_data );

parameter WIDTH = 64;

input clock;
input strobe;
input chan_sel;
input signed [WIDTH-1:0] in_data;
output reg signed [WIDTH-1:0] out_data;

// Раздельная память для хранения предыдущих значений I и Q
reg signed [WIDTH-1:0] prev_data_I = 0;
reg signed [WIDTH-1:0] prev_data_Q = 0;
wire signed [WIDTH-1:0] current_prev = chan_sel ? prev_data_I : prev_data_Q;

always @(posedge clock)  
  if (strobe) 
    begin
    out_data <= in_data - current_prev;
    if (chan_sel) prev_data_I <= in_data;
    else          prev_data_Q <= in_data;
    end

endmodule


//------------------------------------------------------------------------------
//    Модуль интегратора с поддержкой разделения каналов
//------------------------------------------------------------------------------
module cic_integrator1( clock, strobe, chan_sel, in_data, out_data );

parameter WIDTH = 64;

input clock;
input strobe;
input chan_sel;
input signed [WIDTH-1:0] in_data;
output reg signed [WIDTH-1:0] out_data;

// Раздельные сумматоры для хранения интегралов каналов I и Q
reg signed [WIDTH-1:0] out_data_I = 0;
reg signed [WIDTH-1:0] out_data_Q = 0;

always @(posedge clock) begin
  if (strobe) begin
    if (chan_sel) begin
      out_data_I <= out_data_I + in_data;
      out_data <= out_data_I + in_data;
    end else begin
      out_data_Q <= out_data_Q + in_data;
      out_data <= out_data_Q + in_data;
    end
  end
end

endmodule

/*
module cic( clock, in_strobe, out_strobe, in_data, out_data );

//design parameters
parameter STAGES = 3;
parameter DECIMATION = 16;  
parameter IN_WIDTH = 18;

//computed parameters

//parameter ACC_WIDTH = IN_WIDTH + Ceil(STAGES * Log2(DECIMATION))
//parameter OUT_WIDTH = IN_WIDTH + Ceil(Log2(DECIMATION) / 2)
parameter ACC_WIDTH = 30;
parameter OUT_WIDTH = 24;

input clock;
input in_strobe;
output reg out_strobe;
input signed [IN_WIDTH-1:0] in_data;
output signed [OUT_WIDTH-1:0] out_data;

//------------------------------------------------------------------------------
//                               control
//------------------------------------------------------------------------------
reg [15:0] sample_no;
initial sample_no = 15'd0;


always @(posedge clock)
  if (in_strobe)
    begin
    if (sample_no == (DECIMATION-1))
      begin
      sample_no <= 0;
      out_strobe <= 1;
      end
    else
      begin
      sample_no <= sample_no + 8'd1;
      out_strobe <= 0;
      end
    end

  else
    out_strobe <= 0;


//------------------------------------------------------------------------------
//                                stages
//------------------------------------------------------------------------------
wire signed [ACC_WIDTH-1:0] integrator_data [0:STAGES];
wire signed [ACC_WIDTH-1:0] comb_data [0:STAGES];


assign integrator_data[0] = in_data;
assign comb_data[0] = integrator_data[STAGES];


genvar i;
generate
  for (i=0; i<STAGES; i=i+1)
    begin : cic_stages

    cic_integrator1 #(ACC_WIDTH) cic_integrator_inst(
      .clock(clock),
      .strobe(in_strobe),
      .in_data(integrator_data[i]),
      .out_data(integrator_data[i+1])
      );


    cic_comb1 #(ACC_WIDTH) cic_comb_inst(
      .clock(clock),
      .strobe(out_strobe),
      .in_data(comb_data[i]),
      .out_data(comb_data[i+1])
      );
    end
endgenerate







//------------------------------------------------------------------------------
//                            output rounding
//------------------------------------------------------------------------------
assign out_data = comb_data[STAGES][ACC_WIDTH-1:ACC_WIDTH-OUT_WIDTH] +
  {{(OUT_WIDTH-1){1'b0}}, comb_data[STAGES][ACC_WIDTH-OUT_WIDTH-1]};

//assign out_data = comb_data[STAGES][36:19] + comb_data[STAGES][18];


endmodule

module cic_comb1( clock, strobe,  in_data,  out_data );

parameter WIDTH = 64;

input clock;
input strobe;
input signed [WIDTH-1:0] in_data;
output reg signed [WIDTH-1:0] out_data;


reg signed [WIDTH-1:0] prev_data;
initial prev_data = 0;


always @(posedge clock)  
  if (strobe) 
    begin
    out_data <= in_data - prev_data;
    prev_data <= in_data;
    end



endmodule


module cic_integrator1( clock, strobe, in_data,  out_data );

parameter WIDTH = 64;

input clock;
input strobe;
input signed [WIDTH-1:0] in_data;
output reg signed [WIDTH-1:0] out_data;


//initial out_data = 0; // this is NOT a valid RTL statement! Kirk Weedman KD7IRS


always @(posedge clock)
  if (strobe) out_data <= out_data + in_data;


endmodule
*/