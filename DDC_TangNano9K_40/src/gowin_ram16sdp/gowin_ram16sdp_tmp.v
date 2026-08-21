//Copyright (C)2014-2022 Gowin Semiconductor Corporation.
//All rights reserved.
//File Title: Template file for instantiation
//GOWIN Version: V1.9.8.09
//Part Number: GW2AR-LV18QN88C8/I7
//Device: GW2AR-18
//Created Time: Tue Dec 10 13:25:28 2024

//Change the instance name and port connections to the signal names
//--------Copy here to design--------

    Gowin_RAM16SDP your_instance_name(
        .dout(dout_o), //output [35:0] dout
        .wre(wre_i), //input wre
        .wad(wad_i), //input [5:0] wad
        .di(di_i), //input [35:0] di
        .rad(rad_i), //input [5:0] rad
        .clk(clk_i) //input clk
    );

//--------Copy end-------------------
