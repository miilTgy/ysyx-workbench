package wbu

import chisel3._

class WBU extends Module {
    val MUXio = IO(new Bundle {
        val dataWB = Input(UInt(64.W))
    })
    val ioGPR = IO(new Bundle {
        val dataWB = Output(UInt(64.W))
    })
    ioGPR.dataWB := MUXio.dataWB
}
