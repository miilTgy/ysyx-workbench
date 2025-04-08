package imem

import chisel3._
import chisel3.util.HasBlackBoxPath

class IFUIO extends Bundle {
    val pc = Input(UInt(64.W))
}

class IOIDU extends Bundle {
    val pc = Output(UInt(64.W))
    val inst_data = Output(UInt(32.W))
}

class IMEM extends BlackBox with HasBlackBoxPath {
    val io = IO(new Bundle{
        val pc = Input(UInt(64.W))
        val inst_data = Output(UInt(32.W))
    })

    // Set the resource path for the Verilog file
    addPath("/home/miil/ysyx-workbench/npc/vsrc/chisel/IMEM/src/IMEM.v")
}

class IMEM_d extends Module {
    val IFUio = IO(new IFUIO)
    val ioIDU = IO(new IOIDU)

    ioIDU.pc := IFUio.pc

    val imem = Module(new IMEM())
    imem.io.pc := IFUio.pc
    ioIDU.inst_data := imem.io.inst_data
}
