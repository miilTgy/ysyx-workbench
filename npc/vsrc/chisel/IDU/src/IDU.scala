package idu

import java.io._

import chisel3._
import chisel3.util.BitPat
import chisel3.util.experimental.decode._
import org.chipsalliance.rvdecoderdb


case class Insn(val inst: rvdecoderdb.Instruction) extends DecodePattern {
    override def bitPat: BitPat = BitPat("b" + inst.encoding.toString())
}

object isAddi extends BoolDecodeField[Insn] {
    override def name = "is addi"

    override def default = BitPat(false.B)

    override def genTable(i: Insn): BitPat = i.inst.name match {
        case "addi" => BitPat(true.B)
        case _      => BitPat(false.B)
    }
}

class IDU extends Module {
    val io = IO(new Bundle{
        val inst   = Input(UInt(32.W))
        val isAddi = Output(Bool())
    })

    val instTable: Iterable[rvdecoderdb.Instruction] =
            rvdecoderdb.instructions(os.pwd / "rvdecoderdb" / "rvdecoderdbtest" / "jvm" / "riscv-opcodes")
            

    val targetSets = Set("rv_i", "rv64_i", "rv_m", "rv64_m")

    /* OutPut Inst Table Begin */
        val instTableOutputFile = new File("InstSupported.md")
        val rv32imInstListString = instTable
            .filter(instr => targetSets.contains(instr.instructionSet.name)) // filter Sets
            .filter(_.pseudoFrom.isEmpty)

        val writer = new BufferedWriter(new FileWriter(instTableOutputFile))
        writer.write(rv32imInstListString.toString())
        writer.close()
    /* OutPut Inst Table End */

    val rv32imInstList = instTable
        .filter(instr => targetSets.contains(instr.instructionSet.name)) // filter Sets
        .filter(_.pseudoFrom.isEmpty)
        .map(Insn(_))
        .toSeq


    val decodeTable = new DecodeTable(rv32imInstList, Seq(isAddi))

    val decodeResult = decodeTable.decode(io.inst)
    io.isAddi := decodeResult(isAddi)

/*     val imm_i      = Cat(Fill(52, inst(31)), inst(31, 20))                              // I-type
    val imm_s      = Cat(Fill(52, inst(31)), inst(31, 25), inst(11, 7))                 // S-type
    val imm_b      = Cat(Fill(52, inst(31)), inst(7), inst(30, 25), inst(11, 8), 0.U)   // B-type
    val imm_u      = Cat(Fill(32, inst(31)), inst(31, 12), Fill(12, 0.U))               // U-type
    val imm_j      = Cat(Fill(44, inst(31)), inst(19, 12), inst(20), inst(30, 21), 0.U) // J-type
    val imm_shamtd = Cat(Fill(58, 0.U), inst(25, 20))
    val imm_shamtw = Cat(Fill(59, 0.U), inst(24, 20))
 */}