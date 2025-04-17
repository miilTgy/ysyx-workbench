package idu

import java.io._

import chisel3._
import chisel3.dontTouch
import chisel3.util.BitPat
import chisel3.util.MuxLookup
import chisel3.util.Cat
import chisel3.util.Fill
import chisel3.util.experimental.decode._
import org.chipsalliance.rvdecoderdb
import org.chipsalliance.rvdecoderdb.Utils

import aluop._
import imem.IOIDU
import ifu.IOIMEM
import gpr.GPRINIO


case class Insn(val inst: rvdecoderdb.Instruction) extends DecodePattern {
    override def bitPat: BitPat = BitPat("b" + inst.encoding.toString())
}

object aluD1slct extends BoolDecodeField[Insn] {
    override def name = "src1 or pc select mux ctrl"

    override def genTable(i: Insn): BitPat = {
        if (i.inst.name == "auipc" || Utils.isJ(i.inst)) {
            BitPat(true.B)
        } else {
            BitPat(false.B)
        }
    }
}

object aluD2slct extends BoolDecodeField[Insn] {
    override def name = "src2 or imm select mux ctrl"

    override def genTable(i: Insn): BitPat = {
        if (Utils.readRs2(i.inst) && !Utils.isS(i.inst)) {
            BitPat(true.B)
        } else {
            BitPat(false.B)
        }
    }
}


object memEnslct extends BoolDecodeField[Insn] {
    override def name = "dmem pass select mux ctrl"

    val memInst: Seq[String] = Seq(
        "lb", "lh", "lw", "ld",
        "lbu", "lhu", "lwu"
    )
    override def genTable(i: Insn): BitPat = {
        if (memInst.contains(i.inst.name) || Utils.isS(i.inst)) {
            BitPat(true.B)
        } else {
            BitPat(false.B)
        }
    }
}

object JMPslct extends BoolDecodeField[Insn] {
    override def name = "pc inc select mux ctrl"

    override def genTable(i: Insn): BitPat = {
        if ( Utils.isJ(i.inst) ||
             Utils.isB(i.inst) ||
             i.inst.name == "jalr" ) {
            BitPat(true.B)
        } else {
            BitPat(false.B)
        }
    }
}

object PCInc4slct extends BoolDecodeField[Insn] {
    override def name = "pc inc 4 or imm select mux ctrl"

    override def genTable(i: Insn): BitPat = {
        if (i.inst.name == "jal" || i.inst.name == "jalr") {
            BitPat(true.B)
        } else {
            BitPat(false.B)
        }
    }
}

object PCSrcslct extends BoolDecodeField[Insn] {
    override def name = "pc or src select mux ctrl"

    override def genTable(i: Insn): BitPat = {
        if (i.inst.name == "jalr") {
            BitPat(true.B)
        } else {
            BitPat(false.B)
        }
    }
}

object CondReslct extends BoolDecodeField[Insn] {
    override def name = "cond result select mux ctrl"

    val InstSeq: Seq[String] = Seq(
        "slti", "sltiu", "slt", "sltu"
    )
    override def genTable(i: Insn): BitPat = {
        if (InstSeq.contains(i.inst.name)) {
            BitPat(true.B)
        } else {
            BitPat(false.B)
        }
    }
}

object GenSub extends BoolDecodeField[Insn] {
    override def name = "gen branch sign"

    override def genTable(i: Insn): BitPat = {
        if (aluop.AluOpMap.SubSeq.contains(i.inst.name) ||
            aluop.AluOpMap.unSubSeq.contains(i.inst.name) ||
            aluop.AluOpMap.BeqSeq.contains(i.inst.name) ||
            aluop.AluOpMap.BneSeq.contains(i.inst.name) ||
            aluop.AluOpMap.BltSeq.contains(i.inst.name) ||
            aluop.AluOpMap.BgeSeq.contains(i.inst.name)) {
            BitPat(true.B)
        } else {
            BitPat(false.B)
        }
    }
}

object GenWen extends BoolDecodeField[Insn] {
    override def name = "gen wen"

    override def genTable(i: Insn): BitPat = {
        if (Utils.writeRd(i.inst)) {
            BitPat(true.B)
        } else {
            BitPat(false.B)
        }
    }
}

object GenAluOp extends DecodeField[Insn, UInt] {
    override def name = "gen alu op"

    override def chiselType = UInt(4.W)

    override def genTable(op: Insn): BitPat = {
        val aluOp = aluop.AluOpMap.getAluOp(op.inst.name)
        BitPat(aluOp.litValue.U((aluOp.getWidth).W))
    }
}

object GenMwen extends BoolDecodeField[Insn] {
    override def name = "gen mwen"

    override def genTable(i: Insn): BitPat = {
        if (Utils.isS(i.inst)) {
            BitPat(true.B)
        } else {
            BitPat(false.B)
        }
    }
}

object GenWriteMask extends DecodeField[Insn, UInt] {
    override def name = "gen write mask"

    override def chiselType = UInt(9.W)

    override def genTable(i: Insn): BitPat = i.inst.name match {
        case "sb"   => BitPat("b00000001".U(8.W))
        case "sh"   => BitPat("b00000011".U(8.W))
        case "sw"   => BitPat("b00001111".U(8.W))
        case "sd"   => BitPat("b11111111".U(8.W))
        case _      => BitPat("b00000000".U(8.W))
    }
}

object GenSextPos extends DecodeField[Insn, UInt] {
    override def name = "gen sext pos"

    override def chiselType = UInt(2.W)

    override def genTable(i: Insn): BitPat = i.inst.name match {
            case "lb"   => BitPat(0.U(2.W))
            case "lh"   => BitPat(1.U(2.W))
            case "lw"   => BitPat(2.U(2.W))
            case "ld"   => BitPat(3.U(2.W))
            case _      => BitPat(3.U(2.W))
    }
}

object GenAluSext extends DecodeField[Insn, Bool] {
    override def name = "gen alu sext"

    val AluSextSeq: Seq[String] = Seq(
        "addiw", "slliw", "srliw", "sraiw",
        "addw", "subw", "sllw", "srlw", "sraw",
        "mulw", "divw", "divuw", "remw", "remuw"
    )

    override def genTable(i: Insn): BitPat = {
        if (AluSextSeq.contains(i.inst.name)) {
            BitPat(true.B)
        } else {
            BitPat(false.B)
       }
    }

}

object ImmTypeEnum extends ChiselEnum {
    val immNone, immI, immS, immB, immU, immJ = Value
}

object ImmType extends DecodeField[Insn, ImmTypeEnum.Type] {
    override def name = "decode imm type eg. isbuj"

    override def chiselType = ImmTypeEnum()
    
    def isIshamt(instruction: rvdecoderdb.Instruction): Boolean = {
        instruction.args.map(_.name) == Seq("rd", "rs1", "shamtd") ||
        instruction.args.map(_.name) == Seq("rd", "rs1", "shamtw")
    }
    override def genTable(i: Insn): BitPat = {
        val immType = i match {
            case m if Utils.isI(m.inst)  => ImmTypeEnum.immI
            case m if Utils.isS(m.inst)  => ImmTypeEnum.immS
            case m if Utils.isB(m.inst)  => ImmTypeEnum.immB
            case m if Utils.isU(m.inst)  => ImmTypeEnum.immU
            case m if Utils.isJ(m.inst)  => ImmTypeEnum.immJ
            case m if isIshamt(m.inst)   => ImmTypeEnum.immI
            case _                       => ImmTypeEnum.immNone
        }
        // println("BITPAT: " + immType.litValue.U + "width=" + immType.getWidth + " " + "name=" + i.inst.name)
        BitPat(immType.litValue.U((immType.getWidth).W))
    }
}

class IOALU extends Bundle {
    val aluop = Output(UInt(4.W))
    val sub = Output(Bool())
    val condReslct = Output(Bool())
    val Sext = Output(Bool())
}

class IOJMP extends Bundle {
    val jmpslct = Output(Bool())
}

class IODMEM extends Bundle {
    val menslct = Output(Bool())
    val mwen   = Output(Bool())
    val wmask   = Output(UInt(8.W))
    val sextPos = Output(UInt(2.W))
}

class IOWBU extends Bundle {
}

class IDU extends Module {
    val IFUio = IO(Flipped(new ifu.IOIMEM))
    val ioPCs = IO(new ifu.IOIMEM)
    val IMEMio = IO(Flipped(new imem.IOIDU))
    val ioMUX = IO(new Bundle{
        val alud1slct = Output(Bool())
        val alud2slct = Output(Bool())
        val pcInc4slct = Output(Bool())
        val pcsrcslct = Output(Bool())
        val imm       = Output(UInt(64.W))
    })
    val ioALU = IO(new IOALU)
    val ioGPR = IO(Flipped(new gpr.GPRINIO))
    val ioJMP = IO(new IOJMP)
    val ioDMEM = IO(new IODMEM)
    val ioWBU = IO(new IOWBU)

    ioPCs <> IFUio // pc pass through

    val instTable: Iterable[rvdecoderdb.Instruction] =
            rvdecoderdb.instructions(os.pwd / "rvdecoderdb" / "rvdecoderdbtest" / "jvm" / "riscv-opcodes")
            

    val targetSets = Set("rv_i", "rv64_i", "rv_m", "rv64_m")

    /* OutPut Inst Table Begin */
        val instTableOutputFile = new File("/home/miil/ysyx-workbench/npc/vsrc/chisel/InstSupported.md")
        val rv32imInstListString = instTable
            .filter(instr => targetSets.contains(instr.instructionSet.name)) // filter Sets
            .filter(_.pseudoFrom.isEmpty)
            // .filter(instr => instr.args.exists(_.name.contains("imm")))

        val writer = new BufferedWriter(new FileWriter(instTableOutputFile))
        writer.write(rv32imInstListString.toString())
        writer.close()
    /* OutPut Inst Table End */

    val rv32imInstList = instTable
        .filter(instr => targetSets.contains(instr.instructionSet.name)) // filter Sets
        .filter(_.pseudoFrom.isEmpty)
        .map(Insn(_))
        .toSeq


    /* 很玄学的bug：当 ImmType 放在 Seq 中最后一位时，会导致 decodeResult 的值错误！ */
    val decodeTable = new DecodeTable(rv32imInstList, Seq(
        ImmType, aluD1slct, aluD2slct, memEnslct, JMPslct, PCInc4slct, PCSrcslct, CondReslct,
        GenSub, GenWen, GenAluOp, GenSextPos, GenAluSext,
        GenMwen, GenWriteMask
        ))

    val decodeResult = decodeTable.decode(IMEMio.inst_data)

    ioMUX.alud1slct  := decodeResult(aluD1slct)
    ioMUX.alud2slct  := decodeResult(aluD2slct)
    ioMUX.pcInc4slct := decodeResult(PCInc4slct)
    ioMUX.pcsrcslct := decodeResult(PCSrcslct)
    ioALU.aluop      := decodeResult(GenAluOp)
    ioALU.sub := decodeResult(GenSub)
    ioALU.condReslct := decodeResult(CondReslct)
    ioALU.Sext := decodeResult(GenAluSext)
    ioJMP.jmpslct    := decodeResult(JMPslct)
    ioDMEM.menslct   := decodeResult(memEnslct)
    ioDMEM.mwen      := decodeResult(GenMwen)
    ioDMEM.wmask     := decodeResult(GenWriteMask)
    ioDMEM.sextPos    := decodeResult(GenSextPos)

    val imm_i    = Cat(Fill(52, IMEMio.inst_data(31)), IMEMio.inst_data(31, 20))                                                       // I-type
    val imm_s    = Cat(Fill(52, IMEMio.inst_data(31)), IMEMio.inst_data(31, 25), IMEMio.inst_data(11, 7))                              // S-type
    val imm_b    = Cat(Fill(52, IMEMio.inst_data(31)), IMEMio.inst_data(7), IMEMio.inst_data(30, 25), IMEMio.inst_data(11, 8), 0.U)    // B-type
    val imm_u    = Cat(Fill(32, IMEMio.inst_data(31)), IMEMio.inst_data(31, 12), Fill(12, 0.U))                                        // U-type
    val imm_j    = Cat(Fill(44, IMEMio.inst_data(31)), IMEMio.inst_data(19, 12), IMEMio.inst_data(20), IMEMio.inst_data(30, 21), 0.U)  // J-type
    // dontTouch(imm_i)
    // dontTouch(imm_s)
    // dontTouch(imm_b)
    // dontTouch(imm_u)
    // dontTouch(imm_j)
    val imm_type = decodeResult(ImmType)
    // dontTouch(imm_type)

    ioMUX.imm := MuxLookup(imm_type, 0.U)(
        Seq(
            ImmTypeEnum.immNone  -> 0.U,
            ImmTypeEnum.immI    -> imm_i,
            ImmTypeEnum.immS    -> imm_s,
            ImmTypeEnum.immB    -> imm_b,
            ImmTypeEnum.immU    -> imm_u,
            ImmTypeEnum.immJ    -> imm_j
    ))
    
    ioGPR.src1 := IMEMio.inst_data(19, 15)
    ioGPR.src2 := IMEMio.inst_data(24, 20)
    ioGPR.rd   := IMEMio.inst_data(11, 7)
    ioGPR.wen  := decodeResult(GenWen)
}