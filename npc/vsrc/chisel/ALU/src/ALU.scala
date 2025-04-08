package alu

import chisel3._
import chisel3.util.MuxLookup

import aluop._

class ALU extends Module {
    val IDUio = IO(new Bundle{
        val aluop = Input(UInt(4.W))
    })
    val GPRio = IO(new Bundle{
        val data1 = Input(UInt(64.W))
    })
    val MUXio = IO(new Bundle{
        val data2 = Input(UInt(64.W))
    })
    val ioMEMWB = IO(new Bundle{
        val res_addr = Output(UInt(64.W))
    })

    val addRes = GPRio.data1 + MUXio.data2
    val subRes = GPRio.data1 - MUXio.data2
    val xorRes = GPRio.data1 ^ MUXio.data2
    val orRes  = GPRio.data1 | MUXio.data2
    val andRes = GPRio.data1 & MUXio.data2
    val sllRes = GPRio.data1 << MUXio.data2(5,0)
    val srlRes = GPRio.data1 >> MUXio.data2(5,0)
    val sraRes = (GPRio.data1.asSInt >> MUXio.data2(5,0)).asUInt
    val mulRes = /* IDUio.data1 * IDUio.data2 */ 0.U
    val divRes = /* IDUio.data1 / IDUio.data2 */ 0.U
    val remRes = /* IDUio.data1 % IDUio.data2 */ 0.U

    val (aluopDecoded: aluop.AluOp.Type, valid: Bool) = aluop.AluOp.safe(IDUio.aluop)
    assert(valid, "ALU.scala: aluop decode result may be invalid");

    ioMEMWB.res_addr := MuxLookup(aluopDecoded, 0.U)(
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
}