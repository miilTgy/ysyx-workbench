package idu

import chisel3._
import chisel3.dontTouch
import chisel3.util.BitPat
import chisel3.util.MuxLookup
import chisel3.util.Cat
import chisel3.util.Fill
import chisel3.util.experimental.decode._
import org.chipsalliance.rvdecoderdb
import org.chipsalliance.rvdecoderdb.Utils

object CSRInsts {
    val csrInsts = Seq(
        "csrrw", "csrrs",
    )
}

object CSRWenslct extends BoolDecodeField[Insn] {
    override def name = "is csr inst"

    override def genTable(i: Insn): BitPat = {
        if (CSRInsts.csrInsts.contains(i.inst.name)) {
            BitPat(true.B)
        } else {
            BitPat(false.B)
        }
    }
}

object EXPCslct extends BoolDecodeField[Insn] {
    override def name = "is expc inst"

    val InstSeq: Seq[String] = Seq(
        "ecall", "mret"
    )
    override def genTable(i: Insn): BitPat = {
        if (InstSeq.contains(i.inst.name)) {
            BitPat(true.B)
        } else {
            BitPat(false.B)
        }
    }
}

object GenCSROp extends DecodeField[Insn, UInt] {
    override def name = "gen csrop"

    override def chiselType = UInt(csrop.CSROp.getWidth.W)

    override def genTable(op: Insn): BitPat = {
        val csrOp = csrop.CSROpMap.getCSROp(op.inst.name)
        BitPat(csrOp.litValue.U((csrOp.getWidth).W))
    }
}