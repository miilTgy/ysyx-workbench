package dmem

import chisel3._
import chisel3.util.HasBlackBoxResource

class DMEM extends BlackBox with HasBlackBoxResource {
    val io = IO(new Bundle {
        val dm_raddr = Input(UInt(64.W))
        val data = Output(UInt(32.W))
})

    // Set the resource path for the Verilog file
    addResource("/DMEM.v")
}
