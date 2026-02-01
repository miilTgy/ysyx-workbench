package cpu

import chisel3._
import chisel3.dontTouch
import chisel3.util.BitPat
import chisel3.util.MuxLookup
import chisel3.util.Cat
import chisel3.util.Fill
import chisel3.util.experimental.decode._
import org.chipsalliance.rvdecoderdb
import org.chipsalliance.rvdecoderdb.Utils
import cpu.InstPattern

// Decode fields for instructions
object memEnsel extends BoolDecodeField[InstPattern] {
    override def name = "dmem pass select mux ctrl"

    val memInst: Seq[String] = Seq(
        "lb", "lh", "lw", "ld",
        "lbu", "lhu", "lwu"
    )
    override def genTable(op: InstPattern): BitPat = {
        if (memInst.contains(op.inst.name) || Utils.isS(op.inst)) {
            BitPat(true.B)
        } else {
            BitPat(false.B)
        }
    }
}
