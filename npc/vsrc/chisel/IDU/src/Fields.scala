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

import aluop._

object aluD1slct extends BoolDecodeField[Insn] {
    override def name = "src1 or pc select mux ctrl"

    override def genTable(i: Insn): BitPat = {
        if (i.inst.name == "auipc" || Utils.isJ(i.inst) || i.inst.name == "jalr") {
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
            aluop.AluOpMap.BltuSeq.contains(i.inst.name) ||
            aluop.AluOpMap.BgeSeq.contains(i.inst.name) ||
            aluop.AluOpMap.BgeuSeq.contains(i.inst.name)) {
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

    override def chiselType = UInt((AluOp.ADDSUB.getWidth).W)

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

    override def chiselType = UInt(8.W)

    override def genTable(i: Insn): BitPat = i.inst.name match {
        case "sb"   => BitPat("b00000001".U(8.W))
        case "sh"   => BitPat("b00000011".U(8.W))
        case "sw"   => BitPat("b00001111".U(8.W))
        case "sd"   => BitPat("b11111111".U(8.W))
        case _      => BitPat("b00000000".U(8.W))
    }
}

object GenDMEMExtPos extends DecodeField[Insn, UInt] {
    override def name = "gen sext pos"

    override def chiselType = UInt(2.W)

    override def genTable(i: Insn): BitPat = i.inst.name match {
            case "lb"   => BitPat(0.U(2.W))
            case "lbu"  => BitPat(0.U(2.W))
            case "lh"   => BitPat(1.U(2.W))
            case "lhu"  => BitPat(1.U(2.W))
            case "lw"   => BitPat(2.U(2.W))
            case "lwu"  => BitPat(2.U(2.W))
            case "ld"   => BitPat(3.U(2.W))
            case _      => BitPat(3.U(2.W))
    }
}

object DMEMExtSign extends BoolDecodeField[Insn] {
    override def name = "dmem ext sign"

    val SignSeq = Seq("lb", "lh", "lw", "ld")
    
    override def genTable(i: Insn): BitPat = {
        if (SignSeq.contains(i.inst.name)) {
            BitPat(true.B)
        } else {
            BitPat(false.B)
        }
    }
}

object GenAluSext extends BoolDecodeField[Insn] {
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


