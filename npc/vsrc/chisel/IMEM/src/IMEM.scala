package imem

import chisel3._
import chisel3.util.HasBlackBoxPath

class IMEMIO extends Bundle {
    val im_raddr = Input(UInt(64.W))
    val inst_data = Output(UInt(32.W))
}

class IMEM extends BlackBox with HasBlackBoxPath {
    val io = IO(new IMEMIO)

    // Set the resource path for the Verilog file
    addPath("/home/miil/ysyx-workbench/npc/vsrc/chisel/IMEM/src/IMEM.v")
}

class IMEM_d extends Module {
    val io = IO(new IMEMIO)

    val imem = Module(new IMEM())
    imem.io <> io
}
