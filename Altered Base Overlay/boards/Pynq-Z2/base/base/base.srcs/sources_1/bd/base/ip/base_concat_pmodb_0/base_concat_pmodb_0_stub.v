// Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2018.2 (win64) Build 2258646 Thu Jun 14 20:03:12 MDT 2018
// Date        : Wed May 15 15:25:41 2019
// Host        : EEE-R448-19 running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub
//               C:/Users/lpb16134/PYNQ-2.3/boards/Pynq-Z2/base/base/base.srcs/sources_1/bd/base/ip/base_concat_pmodb_0/base_concat_pmodb_0_stub.v
// Design      : base_concat_pmodb_0
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7z020clg400-1
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* X_CORE_INFO = "xlconcat_v2_1_1_xlconcat,Vivado 2018.2" *)
module base_concat_pmodb_0(In0, In1, In2, In3, dout)
/* synthesis syn_black_box black_box_pad_pin="In0[7:0],In1[7:0],In2[7:0],In3[7:0],dout[31:0]" */;
  input [7:0]In0;
  input [7:0]In1;
  input [7:0]In2;
  input [7:0]In3;
  output [31:0]dout;
endmodule
