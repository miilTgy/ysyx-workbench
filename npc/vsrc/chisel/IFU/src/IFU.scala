package ifu

import chisel3._

class IFU extends Module {
    val ioIMEM = IO(new Bundle {
        val pc = Output(UInt(64.W))
    })

    val pc = RegInit("h80000000".U(64.W)) // Hex lit value from string

    pc := pc + 4.U

    ioIMEM.pc := pc
}