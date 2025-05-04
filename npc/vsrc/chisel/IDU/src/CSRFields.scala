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