//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         GAIN Control
//используется для управления входным аттенюатором(при наличии)
//------------------------------------------------------------------------------------------------------------------------------------------------------------

module gain(
input reset,
input clock,
input  [11:0] data,
input  [4:0] att,
input randomize,
input dither,
output [5:0] p_att
);

wire rxnearclip = (data[11:8] == 4'b0111) | (data[11:8] == 4'b1000);
wire rxgoodlvlp = (data[11:9] == 3'b011);
wire rxgoodlvln = (data[11:9] == 3'b100);

reg agc_nearclip;
reg agc_goodlvl;
reg [25:0] agc_delaycnt;
reg [5:0] agc_value;
wire agc_clrnearclip;
wire agc_clrgoodlvl;

always @(posedge clock)
begin
    if (agc_clrnearclip) agc_nearclip <= 1'b0;
    else if (rxnearclip) agc_nearclip <= 1'b1;
end

always @(posedge clock)
begin
    if (agc_clrgoodlvl) agc_goodlvl <= 1'b0;
    else if (rxgoodlvlp | rxgoodlvln) agc_goodlvl <= 1'b1;
end

always @(posedge clock)
begin
    agc_delaycnt <= agc_delaycnt + 26'b1;
end

always @(posedge clock)
begin
    if (reset) 
        agc_value <= 6'b011111;
    // Decrease gain if near clip seen
    else if ((agc_clrnearclip & agc_nearclip & (agc_value != 6'b000000)) | agc_value > gain_value )
        agc_value <= agc_value - 6'h01;
    // Increase if not in the sweet spot of seeing agc_nearclip
    // But no more than ~26dB (38) as that is the place of diminishing returns re the datasheet
    else if ( agc_clrgoodlvl & ~agc_goodlvl & (agc_value <= gain_value))
        agc_value <= agc_value + 6'h01;
end

// tp = 1.0/61.44e6
// 2**26 * tp = 1.0922 seconds
// PGA settling time is less than 500 ns
// Do decrease possible every 2 us (2**7 * tp)
assign agc_clrnearclip = (agc_delaycnt[6:0] == 7'b1111111);
// Do increase possible every 68 ms, 1us before/after a possible descrease
assign agc_clrgoodlvl = (agc_delaycnt[21:0] == 22'b1011111111111110111111);

wire [5:0] gain_value;
assign gain_value = {~dither, ~att};

assign p_att = randomize ? agc_value : gain_value;

endmodule
