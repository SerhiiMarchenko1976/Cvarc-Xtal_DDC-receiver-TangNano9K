//Copyright (C)2014-2022 Gowin Semiconductor Corporation.
//All rights reserved.
//File Title: Template file for instantiation
//GOWIN Version: V1.9.8.09
//Part Number: GW2AR-LV18QN88C8/I7
//Device: GW2AR-18
//Created Time: Tue Dec 10 13:16:30 2024

//Change the instance name and port connections to the signal names
//--------Copy here to design--------

    Gowin_pROM_fir your_instance_name(
        .dout(dout_o), //output [23:0] dout
        .clk(clk_i), //input clk
        .oce(oce_i), //input oce
        .ce(ce_i), //input ce
        .reset(reset_i), //input reset
        .ad(ad_i) //input [7:0] ad
    );

//--------Copy end-------------------
