package idu

import java.io._

import chisel3._
import chisel3.util.BitPat
import chisel3.util.MuxLookup
import chisel3.util.Cat
import chisel3.util.Fill
import chisel3.util.experimental.decode._
import org.chipsalliance.rvdecoderdb
import org.chipsalliance.rvdecoderdb.Utils


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

object GenAluOp extends DecodeField[Insn, UInt] {
    override def name = "gen alu op"

    override def chiselType = UInt(4.W)

    override def default = BitPat("b1111")

    override def genTable(op: Insn): BitPat = op.inst.name match {
        case "addi" => BitPat("b0000")
        case _      => BitPat("b1111")
    }
}

object ImmTypeEnum extends ChiselEnum {
    val immNone, immI, immS, immB, immU, immJ = Value
}

object ImmType extends DecodeField[Insn, ImmTypeEnum.Type] {
    override def name = "decode imm type eg. isbuj"

    override def chiselType = ImmTypeEnum()
    
    override def genTable(i: Insn): BitPat = {
        val immType = (if(Utils.isI(i.inst)) {
            ImmTypeEnum.immI
        } else if (Utils.isS(i.inst)) {
            ImmTypeEnum.immS
        } else if (Utils.isB(i.inst)) {
            ImmTypeEnum.immB
        } else if (Utils.isU(i.inst)) {
            ImmTypeEnum.immU
        } else if (Utils.isJ(i.inst)) {
            ImmTypeEnum.immJ
        } else {
            ImmTypeEnum.immNone
        })
        
        /* println("BITPAT: " + immType.litValue.U) */
        BitPat(immType.litValue.U((immType.getWidth).W))
    }
}

class IDU extends Module {
    val io = IO(new Bundle{
        val pc     = Input(UInt(64.W))
        val inst   = Input(UInt(32.W))
        val isAddi = Output(Bool())
        val aluop  = Output(UInt(4.W))
        val imm    = Output(UInt(64.W))
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


    val decodeTable = new DecodeTable(rv32imInstList, Seq(isAddi, GenAluOp, ImmType))

    val decodeResult = decodeTable.decode(io.inst)
    io.isAddi := decodeResult(isAddi)
    io.aluop := decodeResult(GenAluOp)

    val imm_i      = Cat(Fill(52, io.inst(31)), io.inst(31, 20))                                    // I-type
    val imm_s      = Cat(Fill(52, io.inst(31)), io.inst(31, 25), io.inst(11, 7))                    // S-type
    val imm_b      = Cat(Fill(52, io.inst(31)), io.inst(7), io.inst(30, 25), io.inst(11, 8), 0.U)   // B-type
    val imm_u      = Cat(Fill(32, io.inst(31)), io.inst(31, 12), Fill(12, 0.U))                     // U-type
    val imm_j      = Cat(Fill(44, io.inst(31)), io.inst(19, 12), io.inst(20), io.inst(30, 21), 0.U) // J-type
    val imm_type = decodeResult(ImmType)
    io.imm := MuxLookup(imm_type, 0.U)(
        Seq(
            ImmTypeEnum.immI    -> imm_i,
            ImmTypeEnum.immS    -> imm_s,
            ImmTypeEnum.immB    -> imm_b,
            ImmTypeEnum.immU    -> imm_u,
            ImmTypeEnum.immJ    -> imm_j
    ))
}