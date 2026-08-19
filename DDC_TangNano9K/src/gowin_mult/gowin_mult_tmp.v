//Copyright (C)2014-2022 Gowin Semiconductor Corporation.
//All rights reserved.
//File Title: Template file for instantiation
//GOWIN Version: V1.9.8.09
//Part Number: GW2AR-LV18QN88C8/I7
//Device: GW2AR-18
//Created Time: Tue Dec 10 12:18:11 2024

//Change the instance name and port connections to the signal names
//--------Copy here to design--------

    Gowin_MULT your_instance_name(
        .dout(dout_o), //output [47:0] dout
        .a(a_i), //input [23:0] a
        .b(b_i), //input [23:0] b
        .ce(ce_i), //input ce
        .clk(clk_i), //input clk
        .reset(reset_i) //input reset
    );

//--------Copy end-------------------
