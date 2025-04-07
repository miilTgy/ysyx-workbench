package dmem

import chisel3._
import chisel3.util.HasBlackBoxResource
import chisel3.util.HasBlackBoxPath

class DMEMIO extends Bundle {
    val dm_raddr = Input(UInt(64.W))
    val dm_waddr = Input(UInt(64.W))
    val wdata = Input(UInt(64.W))
    val rdata = Output(UInt(64.W))
}

class DMEM extends BlackBox with HasBlackBoxPath {
    val io = IO(new Bundle {new DMEMIO})

    // Set the resource path for the Verilog file
    // addResource("/DMEM.v")
    addPath("/home/miil/ysyx-workbench/npc/vsrc/chisel/DMEM/src/DMEM.v")
}

class DMEM_d extends Module {
    val io = IO(new Bundle {new DMEMIO})
    
    val dmem = Module(new DMEM())
    dmem.io <> io
}
