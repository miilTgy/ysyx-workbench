package gpr

import chisel3._

class DATAOUTIO extends Bundle {
    val data1 = Output(UInt(64.W))
    val data2 = Output(UInt(64.W))
}

class DATAINIO extends Bundle {
    val dataWB = Input(UInt(64.W))
}

class GPRINIO extends Bundle {
    val src1 = Input(UInt(5.W))
    val src2 = Input(UInt(5.W))
    val rd   = Input(UInt(5.W))
    val wen  = Input(Bool())
}

class GPR extends Module {
    val GPRio = IO(new GPRINIO)
    val dataOutio = IO(new DATAOUTIO)
    val dataInio = IO(new DATAINIO)

    val regs = Reg(Vec(32, UInt(64.W)))

    dataOutio.data1 := Mux(GPRio.src1 === 0.U, 0.U, regs(GPRio.src1))
    dataOutio.data2 := Mux(GPRio.src2 === 0.U, 0.U, regs(GPRio.src2))

    regs(GPRio.rd) := Mux(GPRio.wen, dataInio.dataWB, regs(GPRio.rd))
}
