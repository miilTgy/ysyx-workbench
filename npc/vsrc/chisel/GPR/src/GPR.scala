package gpr

import chisel3._

import idu.IOGPR

class DATAOUTIO extends Bundle {
    val data1 = Output(UInt(64.W))
    val data2 = Output(UInt(64.W))
}

class DATAINIO extends Bundle {
    val dataWB = Input(UInt(64.W))
}

class GPR extends Module {
    val GPRio = IO(Flipped(new IOGPR))
    val dataOutIO = IO(new DATAOUTIO)
    val dataInIO = IO(new DATAINIO)

    val regs = Reg(Vec(32, UInt(64.W)))

    dataOutIO.data1 := Mux(GPRio.src1 === 0.U, 0.U, regs(GPRio.src1))
    dataOutIO.data2 := Mux(GPRio.src2 === 0.U, 0.U, regs(GPRio.src2))

    when (GPRio.wen) {
        regs(GPRio.rd) := dataInIO.dataWB
    }
}
