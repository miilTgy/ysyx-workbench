package imem

import chisel3._
import chisel3.util.HasBlackBoxResource

class IMEM extends BlackBox with HasBlackBoxResource {
    val io = IO(new Bundle {
    val im_raddr = Input(UInt(64.W))
    val inst_data = Output(UInt(32.W))
})

    // Set the resource path for the Verilog file
    addResource("/IMEM.v")
}