package wbu

import chisel3._

class WBU extends Module {
    val io = IO(new Bundle {
        val dataIn = Input(UInt(64.W))
        val dataOut = Output(UInt(64.W))
    })
    io.dataOut := io.dataIn
}
