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

import aluop._
import imem.IOIDU
import gpr.GPRINIO


case class Insn(val inst: rvdecoderdb.Instruction) extends DecodePattern {
    override def bitPat: BitPat = BitPat("b" + inst.encoding.toString())
}

object aluD2slct extends BoolDecodeField[Insn] {
    override def name = "src2 or imm select mux ctrl"

    // override def default: BitPat = BitPat(false.B)
    override def genTable(i: Insn): BitPat = {
        if (Utils.readRs2(i.inst)) {
            BitPat(true.B)
        } else {
            BitPat(false.B)
        }
    }
}


object memPasslct extends BoolDecodeField[Insn] {
    override def name = "dmem pass select mux ctrl"

    // override def default: BitPat = BitPat(false.B)

    val memInst: Seq[String] = Seq(
        "lb", "lh", "lw", "ld",
        "lbu", "lhu", "lwu",
        "sb", "sh", "sw", "sd" // TODO store insts may not needed
    )
    override def genTable(i: Insn): BitPat = {
        if (memInst.contains(i.inst.name)) {
            BitPat(true.B)
        } else {
            BitPat(false.B)
        }
    }
}

object GenWen extends BoolDecodeField[Insn] {
    override def name = "gen wen"

    // override def default: BitPat = BitPat(false.B)

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

    // override def default = BitPat("b1111")

    override def genTable(op: Insn): BitPat = {
        val aluOp = aluop.AluOpMap.getAluOp(op.inst.name)
        BitPat(aluOp.litValue.U((aluOp.getWidth).W))
    }
}

object ImmTypeEnum extends ChiselEnum {
    val immNone, immI, immS, immB, immU, immJ = Value
}

object ImmType extends DecodeField[Insn, ImmTypeEnum.Type] {
    override def name = "decode imm type eg. isbuj"

    override def chiselType = ImmTypeEnum()
    
    override def genTable(i: Insn): BitPat = {
        val immType = i match {
            case m if Utils.isI(m.inst)  => ImmTypeEnum.immI
            case m if Utils.isS(m.inst)  => ImmTypeEnum.immS
            case m if Utils.isB(m.inst)  => ImmTypeEnum.immB
            case m if Utils.isU(m.inst)  => ImmTypeEnum.immU
            case m if Utils.isJ(m.inst)  => ImmTypeEnum.immJ
            case _                       => ImmTypeEnum.immNone
        }
        
        /* println("BITPAT: " + immType.litValue.U) */
        BitPat(immType.litValue.U((immType.getWidth).W))
    }
}

class IOALU extends Bundle {
    val aluop = Output(UInt(4.W))
}

class IDU extends Module {
    val IMEMio = IO(Flipped(new imem.IOIDU))
    val ioMUX = IO(new Bundle{
        val alud2slct = Output(Bool())
        val mpasslct  = Output(Bool())
        val imm       = Output(UInt(64.W))
    })
    val ioALU = IO(new IOALU)
    val ioGPR = IO(Flipped(new gpr.GPRINIO))

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


    val decodeTable = new DecodeTable(rv32imInstList, Seq(aluD2slct, memPasslct, GenWen, GenAluOp, ImmType))

    val decodeResult = decodeTable.decode(IMEMio.inst_data)

    ioMUX.alud2slct := decodeResult(aluD2slct)
    ioMUX.mpasslct  := decodeResult(memPasslct)
    ioALU.aluop     := decodeResult(GenAluOp)

    val imm_i    = Cat(Fill(52, IMEMio.inst_data(31)), IMEMio.inst_data(31, 20))                                                       // I-type
    val imm_s    = Cat(Fill(52, IMEMio.inst_data(31)), IMEMio.inst_data(31, 25), IMEMio.inst_data(11, 7))                              // S-type
    val imm_b    = Cat(Fill(52, IMEMio.inst_data(31)), IMEMio.inst_data(7), IMEMio.inst_data(30, 25), IMEMio.inst_data(11, 8), 0.U)    // B-type
    val imm_u    = Cat(Fill(32, IMEMio.inst_data(31)), IMEMio.inst_data(31, 12), Fill(12, 0.U))                                        // U-type
    val imm_j    = Cat(Fill(44, IMEMio.inst_data(31)), IMEMio.inst_data(19, 12), IMEMio.inst_data(20), IMEMio.inst_data(30, 21), 0.U)  // J-type
    val imm_type = decodeResult(ImmType)

    ioMUX.imm := MuxLookup(imm_type, 0.U)(
        Seq(
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