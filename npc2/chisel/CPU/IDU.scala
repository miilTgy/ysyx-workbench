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

// All possible instruction patterns
case class InstPattern(val inst: rvdecoderdb.Instruction) extends DecodePattern {
    override def bitPat: BitPat = BitPat("b" + inst.encoding.toString())
}

class IDU extends Module {
    val io = IO(new Bundle {
        val inst = Input(UInt(32.W))
        val memEnsel = Output(Bool())
    })

    val instTable: Iterable[rvdecoderdb.Instruction] =
        rvdecoderdb.instructions(os.pwd / "riscv-opcodes")

    val targetSets = Set("rv_i", "rv64_i", "rv_m", "rv64_m")
    val rv64im: Seq[InstPattern] = instTable
        .filter(instr => targetSets.contains(instr.instructionSet.name)) // filter Sets
        .filter(_.pseudoFrom.isEmpty)
        .map(InstPattern(_))
        .toSeq
    require(
        rv64im.nonEmpty,
        s"Empty decode table. instructionSet names seen: ${instTable.map(_.instructionSet.name).toSet.toSeq.sorted.mkString(", ")}"
    )

    val decodeTable = new DecodeTable(rv64im, Seq(memEnsel))
    val decodeResult = decodeTable.decode(io.inst)
    io.memEnsel := decodeResult(memEnsel)
}
