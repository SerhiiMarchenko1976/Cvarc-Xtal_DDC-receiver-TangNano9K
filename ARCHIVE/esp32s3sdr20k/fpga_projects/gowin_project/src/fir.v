
//------------------------------------------------------------------------------
//           Copyright (c) 2008 Alex Shovkoplyas, VE3NEA
//------------------------------------------------------------------------------



module fir( clock, start, coeff, in_data, out_data, out_strobe, test );

parameter OUT_WIDTH = 32;
localparam MSB = 46;
localparam LSB = MSB - OUT_WIDTH + 1;

input clock;
input start;
input signed [23:0] coeff;
input signed [23:0] in_data;
output reg signed [OUT_WIDTH-1:0] out_data;
output reg out_strobe;
output test;


reg [2:0] state;
reg shift;
reg clear_mac;
reg even_sample;
wire last_sample;
wire [23:0] shr_out;
wire signed [55:0] mac_out;

initial
  begin
  shift = 0;
  clear_mac = 1; 
  state = 0;
  even_sample = 1;
  end

assign test = last_sample;
always @(posedge clock)
    case (state)
      0: //if start=1: write new sample to shiftreg, dump the oldest sample
        if (start) state <= state + 1'b1;

      1: //clear mac
        begin         
        clear_mac <= 1;
        state <= state + 1'b1;
        end
        
      2: //switch shiftreg to shift mode; enable mac
        begin         
        shift <= 1;
        clear_mac <= 0;
        state <= state + 1'b1;
        end
      

      3: //strobe 256 sample/coeff pairs into mac, then stop shifting
        if (last_sample) 
          begin
          shift <= 0;
          state <= state + 1'b1;
          end
          
      4: //wait for the mac pipeline to finish
        state <= state + 1'b1;
      5: //wait for the mac pipeline to finish
        state <= state + 1'b1;
        
      6: //round and register mac output, strobe every even out_data
        begin       
        out_data <= mac_out[MSB:LSB] + mac_out[LSB-1];
        if (even_sample) out_strobe <= 1;
        even_sample <= ~even_sample;
        state <= state + 1'b1;
        end

      7: //done, clear strobe
        begin
        out_strobe <= 0;     
        state <= 0;
        end

    endcase


//------------------------------------------------------------------------------
//                    circular shift register 256 x 24 bit
//------------------------------------------------------------------------------
wire [23:0] shr_in = start ? in_data : shr_out;

    shift_reg shift_r(
        .clock(clock),
        .clken(start | shift),
        .data_in({start, shr_in}), //MSB bit flags the new sample
        .data_out({last_sample, shr_out})
    );



//------------------------------------------------------------------------------
//                        multiplier / accumulator
//------------------------------------------------------------------------------
    fir_mac fir_mac_inst(
        .clock(clock),
        .clear(clear_mac),
        .in_data_1(shr_out),
        .in_data_2(coeff),
        .out_data(mac_out)
    );

endmodule

module fir_mac(
  input clock,
  input clear,
  input signed [23:0] in_data_1,
  input signed [23:0] in_data_2,
  output reg signed [55:0] out_data
  );
wire signed [47:0] product;

//pipelined multiplier: throughput = 1, latency = 3

    Gowin_MULT mult_24Sx24S_inst(
        .dout(product), //output [47:0] dout
        .a(in_data_1), //input [23:0] a
        .b(in_data_2), //input [23:0] b
        .ce(1'b1), //input ce
        .clk(clock), //input clk
        .reset(clear) //input reset
    );
    always @(posedge clock) if (clear) out_data <= 0; else out_data <= out_data + product;

endmodule

module fir_coeffs(
  input clock,
  input start,
  output signed [23:0] coeff
  );

reg [7:0] coeff_idx;

always @(posedge clock)
  if (start) coeff_idx <= 0; 
  else coeff_idx <= coeff_idx + 8'b1;          

    Gowin_pROM_fir fir_coeffs_rom_inst(
        .dout(coeff), //output [23:0] dout
        .clk(clock), //input clk
        .oce(1'b1), //input oce
        .ce(1'b1), //input ce
        .reset(start), //input reset
        .ad(coeff_idx) //input [7:0] ad
    );


endmodule

module shift_reg(
    input clock,
    input clken,
    input  [24:0] data_in,
    output [24:0] data_out
    );

    reg [24:0] ram_reg[0:255];
    reg [7:0]  count;
    initial
        begin
            count = 8'b0;
        end
    always @(posedge clock)
        if(clken)
            begin
                count <= count+8'b1;
                ram_reg[count] <= data_in;
                
            end

    assign data_out = ram_reg[count];
endmodule
