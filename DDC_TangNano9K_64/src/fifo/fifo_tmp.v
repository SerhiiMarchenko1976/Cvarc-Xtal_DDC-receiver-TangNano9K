//Copyright (C)2014-2025 Gowin Semiconductor Corporation.
//All rights reserved.
//File Title: Template file for instantiation
//Tool Version: V1.9.11.02 (64-bit)
//Part Number: GW2AR-LV18QN88C8/I7
//Device: GW2AR-18
//Created Time: Fri Aug 21 21:11:55 2026

//Change the instance name and port connections to the signal names
//--------Copy here to design--------

	fifo your_instance_name(
		.Data(Data), //input [63:0] Data
		.WrReset(WrReset), //input WrReset
		.RdReset(RdReset), //input RdReset
		.WrClk(WrClk), //input WrClk
		.RdClk(RdClk), //input RdClk
		.WrEn(WrEn), //input WrEn
		.RdEn(RdEn), //input RdEn
		.Q(Q), //output [63:0] Q
		.Empty(Empty), //output Empty
		.Full(Full) //output Full
	);

//--------Copy end-------------------
