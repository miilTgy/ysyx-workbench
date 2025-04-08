package ifu

import chisel3._

class IOIMEM extends Bundle {
    val pc = Output(UInt(64.W))
}

class IFU extends Module {
    val ioIMEM = IO(new IOIMEM)

    val pc = RegInit("h80000000".U(64.W)) // Hex Litvalue from string

    pc := pc + 4.U

    ioIMEM.pc := pc
}