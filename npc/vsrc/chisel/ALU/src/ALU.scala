package alu

import chisel3._
import chisel3.util.MuxLookup

import aluop._

class ALU extends Module {
    val ioIDU = IO(new Bundle{
        val pc = Input(UInt(64.W))
        val aluop = Input(UInt(4.W))
        val data1 = Input(UInt(64.W))
        val data2 = Input(UInt(64.W))
    })

    val ioMEMWB = IO(new Bundle{
        val pc = Output(UInt(64.W))
        val result = Output(UInt(64.W))
    })

    val addRes = ioIDU.data1 + ioIDU.data2
    val subRes = ioIDU.data1 - ioIDU.data2
    val xorRes = ioIDU.data1 ^ ioIDU.data2
    val orRes  = ioIDU.data1 | ioIDU.data2
    val andRes = ioIDU.data1 & ioIDU.data2
    val sllRes = ioIDU.data1 << ioIDU.data2(5,0)
    val srlRes = ioIDU.data1 >> ioIDU.data2(5,0)
    val sraRes = (ioIDU.data1.asSInt >> ioIDU.data2(5,0)).asUInt
    val mulRes = /* ioIDU.data1 * ioIDU.data2 */ 0.U
    val divRes = /* ioIDU.data1 / ioIDU.data2 */ 0.U
    val remRes = /* ioIDU.data1 % ioIDU.data2 */ 0.U

    val (aluopDecoded: aluop.AluOp.Type, valid: Bool) = aluop.AluOp.safe(ioIDU.aluop)
    assert(valid, "ALU.scala: aluop decode result may be invalid");

    ioMEMWB.result := MuxLookup(aluopDecoded, 0.U)(
        Seq(
            AluOp.ADD -> addRes,
            AluOp.SUB -> subRes,
            AluOp.XOR -> xorRes,
            AluOp.OR  -> orRes,
            AluOp.AND -> andRes,
            AluOp.SLL -> sllRes,
            AluOp.SLR -> srlRes,
            AluOp.SRA -> sraRes,
            AluOp.MUL -> mulRes,
            AluOp.DIV -> divRes,
            AluOp.REM -> remRes
    ))

    ioMEMWB.pc := ioIDU.pc
}