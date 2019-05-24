// Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2018.2 (win64) Build 2258646 Thu Jun 14 20:03:12 MDT 2018
// Date        : Wed May 15 15:24:30 2019
// Host        : EEE-R448-19 running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub
//               C:/Users/lpb16134/PYNQ-2.3/boards/Pynq-Z2/base/base/base.srcs/sources_1/bd/base/ip/base_color_swap_0_0/base_color_swap_0_0_stub.v
// Design      : base_color_swap_0_0
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7z020clg400-1
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* X_CORE_INFO = "color_swap,Vivado 2018.2" *)
module base_color_swap_0_0(hsync_in, hsync_out, pixel_in, pixel_out, vde_in, 
  vde_out, vsync_in, vsync_out)
/* synthesis syn_black_box black_box_pad_pin="hsync_in,hsync_out,pixel_in[23:0],pixel_out[23:0],vde_in,vde_out,vsync_in,vsync_out" */;
  input hsync_in;
  output hsync_out;
  input [23:0]pixel_in;
  output [23:0]pixel_out;
  input vde_in;
  output vde_out;
  input vsync_in;
  output vsync_out;
endmodule
