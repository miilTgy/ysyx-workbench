package alu

import chisel3._
import chisel3.util.MuxLookup
import chisel3.util.Fill
import chisel3.dontTouch

import aluop._
import idu.{IOALU, IOJMP}
import chisel3.util.Cat

class IODMEMUX extends Bundle {
    val res_addr = Output(UInt(64.W))
}

class ALU extends Module {
    val IDUio = IO(Flipped(new idu.IOALU))
    val MUXio = IO(new Bundle{
        val data1 = Input(UInt(64.W))
        val data2 = Input(UInt(64.W))
    })
    val ioDMEMUX = IO(new IODMEMUX)
    val ioJMP = IO(new idu.IOJMP)
    /* 一：aluop是ADD（jal和jalrn指令）；为高*/
    /* 二：aluop是SUB（B类型指令）&& 比较结果为真；为高*/

    val data2s = (MUXio.data2 ^ Fill(64, IDUio.sub)) + Cat(0.U(63.W), IDUio.sub)
    dontTouch(data2s)

    val addsubTmp = MUXio.data1 + data2s
    dontTouch(addsubTmp)
    val unsignedCond = IDUio.aluop === aluop.AluOp.UNSUB.litValue.U((aluop.AluOp.getWidth).W)
    dontTouch(unsignedCond)
    val condRes = Mux(unsignedCond, (MUXio.data1 < MUXio.data2), (MUXio.data1.asSInt < MUXio.data2.asSInt))
    dontTouch(condRes)
    val addsubRes = Mux(IDUio.condReslct, Cat(Fill(63, 0.U(1.W)), condRes), addsubTmp(63, 0))
    // val subRes = GPRio.data1 - MUXio.data2
    val xorRes = MUXio.data1 ^ MUXio.data2
    val orRes  = MUXio.data1 | MUXio.data2
    val andRes = MUXio.data1 & MUXio.data2
    val sllRes = MUXio.data1 << MUXio.data2(5,0)
    val srlRes = MUXio.data1 >> MUXio.data2(5,0)
    val sraRes = (MUXio.data1.asSInt >> MUXio.data2(5,0)).asUInt
        val sllwLow = Wire(UInt(32.W))
        val sllwHigh = Wire(UInt(32.W))
        sllwLow := MUXio.data1(31, 0) << MUXio.data2(4,0)
        sllwHigh := MUXio.data1(63, 32)
    val sllwRes = Cat(sllwHigh, sllwLow)
        val srlwLow = Wire(UInt(32.W))
        val srlwHigh = Wire(UInt(32.W))
        srlwLow := MUXio.data1(31, 0) >> MUXio.data2(4,0)
        srlwHigh := MUXio.data1(63, 32)
    val srlwRes = Cat(srlwHigh, srlwLow)
        val srawLow = Wire(UInt(32.W))
        val srawHigh = Wire(UInt(32.W))
        srawLow := (MUXio.data1(31, 0).asSInt >> MUXio.data2(4,0)).asUInt
        srawHigh := MUXio.data1(63, 32)
    val srawRes = Cat(srawHigh, srawLow)
    val mulRes = MUXio.data1 * MUXio.data2 /* 0.U */
    // dontTouch(mulRes)
    val divRes = MUXio.data1 / MUXio.data2 /* 0.U */
    val remRes = MUXio.data1 % MUXio.data2 /* 0.U */

    val (aluopDecoded: aluop.AluOp.Type, valid: Bool) = aluop.AluOp.safe(IDUio.aluop)
    assert(valid, "ALU.scala: aluop decode result may be invalid");
    dontTouch(aluopDecoded)

    val ResPreSext = MuxLookup(aluopDecoded, 0.U)(
        Seq(
            // AluOp.ADD -> addsubRes,
            AluOp.ADDSUB -> addsubRes,
            AluOp.UNSUB -> addsubRes,
            // AluOp.SUB -> addsubRes,
            AluOp.XOR -> xorRes,
            AluOp.OR  -> orRes,
            AluOp.AND -> andRes,
            AluOp.SLL -> sllRes,
            AluOp.SLR -> srlRes,
            AluOp.SRA -> sraRes,
            AluOp.MUL -> mulRes,
            AluOp.DIV -> divRes,
            AluOp.REM -> remRes,
            AluOp.BEQ -> addsubRes,
            AluOp.BNE -> addsubRes,
            AluOp.BLT -> addsubRes,
            AluOp.BGE -> addsubRes,
            AluOp.SLLW -> sllwRes,
            AluOp.SRLW -> srlwRes,
            AluOp.SRAW -> srawRes,
            AluOp.D2PASS -> MUXio.data2
    ))
    dontTouch(ResPreSext)
        ioDMEMUX.res_addr := Mux(IDUio.Sext, Cat(Fill(32, ResPreSext(31)), ResPreSext(31, 0)), ResPreSext)

    val branchRes = MuxLookup(aluopDecoded, 0.B)(
        Seq(
            AluOp.BEQ -> (addsubRes === 0.U),
            AluOp.BNE -> (addsubRes =/= 0.U),
            AluOp.BLT -> (Mux(IDUio.sub, addsubRes.asSInt < 0.S, addsubRes.asUInt < 0.U)),
            AluOp.BGE -> (Mux(IDUio.sub, addsubRes.asSInt >= 0.S, addsubRes.asUInt >= 0.U))
        )
    )
    dontTouch(branchRes)
    val branchOper = (IDUio.aluop === aluop.AluOp.BEQ.litValue.U((aluop.AluOp.getWidth).W)) ||
                     (IDUio.aluop === aluop.AluOp.BNE.litValue.U((aluop.AluOp.getWidth).W)) ||
                     (IDUio.aluop === aluop.AluOp.BLT.litValue.U((aluop.AluOp.getWidth).W)) ||
                     (IDUio.aluop === aluop.AluOp.BGE.litValue.U((aluop.AluOp.getWidth).W))
    dontTouch(branchOper)

    ioJMP.jmpslct := (IDUio.aluop === aluop.AluOp.ADD.litValue.U((aluop.AluOp.getWidth).W)) ||
                       (branchOper && branchRes)
}