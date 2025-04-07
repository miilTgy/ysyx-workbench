package dmem

import chisel3._
import chisel3.util.HasBlackBoxResource
import chisel3.util.HasBlackBoxPath

class DMEM extends BlackBox with HasBlackBoxPath {
    val io = IO(new Bundle {
        val dm_raddr = Input(UInt(64.W))
        val data = Output(UInt(32.W))
})

    // Set the resource path for the Verilog file
    // addResource("/DMEM.v")
    addPath("/home/miil/ysyx-workbench/npc/vsrc/chisel/DMEM/src/DMEM.v")
}

class DMEM_d extends Module {
    val io = IO(new Bundle {
        val dm_raddr = Input(UInt(64.W))
        val data = Output(UInt(32.W))
    })

    val dmem = Module(new DMEM())
    dmem.io <> io
}
