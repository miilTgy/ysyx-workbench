package ifu

import chisel3._

class IOIMEM extends Bundle {
    val pc = Output(UInt(64.W))
}

class MUXIO extends Bundle {
    val pcNext = Input(UInt(64.W))
}

class IFU extends Module {
    val ioIMEM = IO(new IOIMEM)
    val MUXio = IO(new MUXIO)

    val pc = RegInit("h80000000".U(64.W)) // Hex Litvalue from string

    pc := MUXio.pcNext

    ioIMEM.pc := pc
}