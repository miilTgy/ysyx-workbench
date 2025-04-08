package dmem

import chisel3._
import chisel3.util.HasBlackBoxResource
import chisel3.util.HasBlackBoxPath

class DMEMIO extends Bundle {
    val addr = Input(UInt(64.W))
    val wdata = Input(UInt(64.W))
    val rdata = Output(UInt(64.W))
}

class DMEM extends BlackBox with HasBlackBoxPath {
    val io = IO(new DMEMIO)

    // Set the resource path for the Verilog file
    // addResource("/DMEM.v")
    addPath("/home/miil/ysyx-workbench/npc/vsrc/chisel/DMEM/src/DMEM.v")
}

class DMEM_d extends Module {
    val ALUio = IO(new Bundle {
        val res_addr = Input(UInt(64.W))
    })
    val GPRio = IO(new Bundle {
        val wdata = Input(UInt(64.W))
    })
    val ioMUX = IO(new Bundle {
        val rdata = Output(UInt(64.W))
    })

    val dmem = Module(new DMEM())
    dmem.io.addr := ALUio.res_addr
    dmem.io.wdata := GPRio.wdata
    ioMUX.rdata := dmem.io.rdata
}
