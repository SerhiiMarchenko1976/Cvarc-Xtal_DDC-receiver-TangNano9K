//Copyright (C)2014-2024 GOWIN Semiconductor Corporation.
//All rights reserved.
//File Title: Timing Constraints file
//GOWIN Version: 1.9.8.09 
//Created Time: 2024-12-10 14:52:10
create_clock -name clk61 -period 16.276 -waveform {0 8} [get_ports {clk61}]
create_clock -name clk96 -period 10.417 -waveform {0 4} [get_ports {clk96}]
create_clock -name spi_sck -period 50 -waveform {0 25} [get_ports {SPI_SCK}]
