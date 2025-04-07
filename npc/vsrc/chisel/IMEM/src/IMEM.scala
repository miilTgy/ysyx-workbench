package imem

import chisel3._
import chisel3.util.HasBlackBoxPath

class IMEM extends BlackBox with HasBlackBoxPath {
    val io = IO(new Bundle {
    val im_raddr = Input(UInt(64.W))
    val inst_data = Output(UInt(32.W))
})

    // Set the resource path for the Verilog file
    addPath("/home/miil/ysyx-workbench/npc/vsrc/chisel/IMEM/src/IMEM.v")
}

class IMEM_d extends Module {
    val io = IO(new Bundle {
        val im_raddr = Input(UInt(64.W))
        val inst_data = Output(UInt(32.W))
    })

    val imem = Module(new IMEM())
    imem.io <> io
}
