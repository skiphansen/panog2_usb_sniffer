//-----------------------------------------------------------------
//                       USB Sniffer Core
//                            V0.5
//                     Ultra-Embedded.com
//                     Copyright 2016-2020
//
//                 Email: admin@ultra-embedded.com
//
//                         License: LGPL
//-----------------------------------------------------------------
//
// This source file may be used and distributed without
// restriction provided that this copyright statement is not
// removed from the file and that any derivative work contains
// the original copyright notice and the associated disclaimer.
//
// This source file is free software; you can redistribute it
// and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any
// later version.
//
// This source is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General
// Public License along with this source; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place, Suite 330,
// Boston, MA  02111-1307  USA
//-----------------------------------------------------------------
module usb_sniffer
(
    // Inputs
     input           clk_i
    ,input           rst_i
    ,input           cfg_awvalid_i
    ,input  [ 31:0]  cfg_awaddr_i
    ,input           cfg_wvalid_i
    ,input  [ 31:0]  cfg_wdata_i
    ,input  [  3:0]  cfg_wstrb_i
    ,input           cfg_bready_i
    ,input           cfg_arvalid_i
    ,input  [ 31:0]  cfg_araddr_i
    ,input           cfg_rready_i
    ,input  [  7:0]  utmi_data_out_i
    ,input  [  7:0]  utmi_data_in_i
    ,input           utmi_txvalid_i
    ,input           utmi_txready_i
    ,input           utmi_rxvalid_i
    ,input           utmi_rxactive_i
    ,input           utmi_rxerror_i
    ,input  [  1:0]  utmi_linestate_i
    ,input           outport_awready_i
    ,input           outport_wready_i
    ,input           outport_bvalid_i
    ,input  [  1:0]  outport_bresp_i
    ,input  [  3:0]  outport_bid_i
    ,input           outport_arready_i
    ,input           outport_rvalid_i
    ,input  [ 31:0]  outport_rdata_i
    ,input  [  1:0]  outport_rresp_i
    ,input  [  3:0]  outport_rid_i
    ,input           outport_rlast_i

    // Outputs
    ,output          cfg_awready_o
    ,output          cfg_wready_o
    ,output          cfg_bvalid_o
    ,output [  1:0]  cfg_bresp_o
    ,output          cfg_arready_o
    ,output          cfg_rvalid_o
    ,output [ 31:0]  cfg_rdata_o
    ,output [  1:0]  cfg_rresp_o
    ,output [  1:0]  utmi_op_mode_o
    ,output [  1:0]  utmi_xcvrselect_o
    ,output          utmi_termselect_o
    ,output          utmi_dppulldown_o
    ,output          utmi_dmpulldown_o
);



//-----------------------------------------------------------------
// Core
//-----------------------------------------------------------------
wire         fifo_tvalid_w;
wire [31:0]  fifo_tdata_w;
wire         fifo_overrun_w;
wire         buffer_full_w;

wire [31:0]  fifo_read_w;
wire [13:0]  fifo_read_available_w;
wire         fifo_read_valid_w;
wire         buffer_reset_w;

usb_sniffer_stream
u_core
(
     .clk_i(clk_i)
    ,.rst_i(rst_i)

    // Config
    ,.cfg_awvalid_i(cfg_awvalid_i)
    ,.cfg_awaddr_i(cfg_awaddr_i)
    ,.cfg_wvalid_i(cfg_wvalid_i)
    ,.cfg_wdata_i(cfg_wdata_i)
    ,.cfg_wstrb_i(cfg_wstrb_i)
    ,.cfg_bready_i(cfg_bready_i)
    ,.cfg_arvalid_i(cfg_arvalid_i)
    ,.cfg_araddr_i(cfg_araddr_i)
    ,.cfg_rready_i(cfg_rready_i)
    ,.cfg_awready_o(cfg_awready_o)
    ,.cfg_wready_o(cfg_wready_o)
    ,.cfg_bvalid_o(cfg_bvalid_o)
    ,.cfg_bresp_o(cfg_bresp_o)
    ,.cfg_arready_o(cfg_arready_o)
    ,.cfg_rvalid_o(cfg_rvalid_o)
    ,.cfg_rdata_o(cfg_rdata_o)
    ,.cfg_rresp_o(cfg_rresp_o)

    // UTMI
    ,.utmi_data_out_i(utmi_data_out_i)
    ,.utmi_data_in_i(utmi_data_in_i)
    ,.utmi_txvalid_i(utmi_txvalid_i)
    ,.utmi_txready_i(utmi_txready_i)
    ,.utmi_rxvalid_i(utmi_rxvalid_i)
    ,.utmi_rxactive_i(utmi_rxactive_i)
    ,.utmi_rxerror_i(utmi_rxerror_i)
    ,.utmi_linestate_i(utmi_linestate_i)
    ,.utmi_op_mode_o(utmi_op_mode_o)
    ,.utmi_xcvrselect_o(utmi_xcvrselect_o)
    ,.utmi_termselect_o(utmi_termselect_o)
    ,.utmi_dppulldown_o(utmi_dppulldown_o)
    ,.utmi_dmpulldown_o(utmi_dmpulldown_o)

    // Stream
    ,.outport_tvalid_o(fifo_tvalid_w)
    ,.outport_tdata_o(fifo_tdata_w)
    ,.outport_tready_i(~fifo_overrun_w)
    // Buffer Config
    ,.buffer_reset_o(buffer_reset_w)
    
    // Read FIFO AXI4 interface
    ,.fifo_read_data_i(fifo_read_w)
    ,.fifo_read_available_i({fifo_overrun_w, 17'b0, fifo_read_available_w})
    ,.fifo_read_valid_o(fifo_read_valid_w)
);

wire fifo_reset_w = rst_i || buffer_reset_w;

capture_fifo
u_capture_fifo
(
// Inputs
    .clk(clk_i)
    ,.rst(fifo_reset_w)
    ,.din(fifo_tdata_w)
    ,.wr_en(fifo_tvalid_w)
    ,.rd_en(fifo_read_valid_w)
// Outputs
    ,.dout(fifo_read_w)
    ,.full()
    ,.overflow(fifo_overrun_w)
    ,.empty()
    ,.data_count(fifo_read_available_w)
);

endmodule
