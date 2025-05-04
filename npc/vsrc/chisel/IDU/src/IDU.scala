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

class IOALU extends Bundle {
    val aluop = Output(UInt(5.W))
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
    val extPos = Output(UInt(2.W))
    val extSign = Output(Bool())
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
    val csrSets = Set("rv_zicsr")
    val csrInstSets = CSRInsts.csrInsts
    val systemSets = Set("rv_system")
    val systemInstSets = Set("mret")

    /* OutPut Inst Table Begin */
        val instTableOutputFile = new File("/home/miil/ysyx-workbench/npc/vsrc/chisel/InstSupported.md")
        var rv32imInstListString = instTable
            .filter(instr => targetSets.contains(instr.instructionSet.name)) // filter Sets
            .filter(_.pseudoFrom.isEmpty)
            
        val rvcsrInstListString = instTable
            .filter(instr => csrSets.contains(instr.instructionSet.name) && csrInstSets.contains(instr.name))
            .filter(_.pseudoFrom.isEmpty)

        val rvsystemInstListString = instTable
            .filter(instr => systemSets.contains(instr.instructionSet.name) && systemInstSets.contains(instr.name))
            .filter(_.pseudoFrom.isEmpty)

        val writer = new BufferedWriter(new FileWriter(instTableOutputFile))
        writer.write((rv32imInstListString ++ rvcsrInstListString ++ rvsystemInstListString).toString())
        writer.close()
    /* OutPut Inst Table End */

    val rv32imInstList = instTable
        .filter(instr => targetSets.contains(instr.instructionSet.name)) // filter Sets
        // .filter(inst => simplecsrSets.contains(inst.name)) // filter names
        .filter(_.pseudoFrom.isEmpty)
        .map(Insn(_))
        .toSeq

    val rvcsrInstList = instTable
        .filter(instr => csrSets.contains(instr.instructionSet.name) && csrInstSets.contains(instr.name))
        .filter(_.pseudoFrom.isEmpty)
        .map(Insn(_))
        .toSeq

    val rvsystemInstList = instTable
        .filter(instr => systemSets.contains(instr.instructionSet.name) && systemInstSets.contains(instr.name))
        .filter(_.pseudoFrom.isEmpty)
        .map(Insn(_))
        .toSeq

    /* 很玄学的bug：当 ImmType 放在 Seq 中最后一位时，会导致 decodeResult 的值错误！ */
    val decodeTable = new DecodeTable(rv32imInstList ++ rvcsrInstList ++ rvsystemInstList,
    Seq(
        ImmType, aluD1slct, aluD2slct, memEnslct,
        JMPslct, PCInc4slct, PCSrcslct, CondReslct, DMEMExtSign,
        GenAluOp, GenSub, GenWen, GenAluSext, GenWriteMask,
        GenDMEMExtPos, GenMwen,
        CSRWenslct
    ))
    val decodeResult = decodeTable.decode(IMEMio.inst_data)

    ioMUX.alud1slct  := decodeResult(aluD1slct)
    ioMUX.alud2slct  := decodeResult(aluD2slct)
    ioMUX.pcInc4slct := decodeResult(PCInc4slct)
    ioMUX.pcsrcslct  := decodeResult(PCSrcslct)
    ioALU.aluop      := decodeResult(GenAluOp)
    ioALU.sub        := decodeResult(GenSub)
    ioALU.condReslct := decodeResult(CondReslct)
    ioALU.Sext       := decodeResult(GenAluSext)
    ioJMP.jmpslct    := decodeResult(JMPslct)
    ioDMEM.menslct   := decodeResult(memEnslct)
    ioDMEM.mwen      := decodeResult(GenMwen)
    ioDMEM.wmask     := decodeResult(GenWriteMask)
    ioDMEM.extPos    := decodeResult(GenDMEMExtPos)
    ioDMEM.extSign   := decodeResult(DMEMExtSign)

    val imm_i    = Cat(Fill(52, IMEMio.inst_data(31)), IMEMio.inst_data(31, 20))                                                       // I-type
    val imm_s    = Cat(Fill(52, IMEMio.inst_data(31)), IMEMio.inst_data(31, 25), IMEMio.inst_data(11, 7))                              // S-type
    val imm_b    = Cat(Fill(52, IMEMio.inst_data(31)), IMEMio.inst_data(7), IMEMio.inst_data(30, 25), IMEMio.inst_data(11, 8), 0.U)    // B-type
    val imm_u    = Cat(Fill(32, IMEMio.inst_data(31)), IMEMio.inst_data(31, 12), Fill(12, 0.U))                                        // U-type
    val imm_j    = Cat(Fill(44, IMEMio.inst_data(31)), IMEMio.inst_data(19, 12), IMEMio.inst_data(20), IMEMio.inst_data(30, 21), 0.U)  // J-type
    dontTouch(imm_i)
    dontTouch(imm_s)
    dontTouch(imm_b)
    dontTouch(imm_u)
    dontTouch(imm_j)
    val imm_type = decodeResult(ImmType)
    dontTouch(imm_type)

    ioMUX.imm := MuxLookup(imm_type.asUInt, 0.U)(
        Seq(
            ImmTypeEnum.immNone.asUInt  -> 0.U,
            ImmTypeEnum.immI.asUInt    -> imm_i,
            ImmTypeEnum.immS.asUInt    -> imm_s,
            ImmTypeEnum.immB.asUInt    -> imm_b,
            ImmTypeEnum.immU.asUInt    -> imm_u,
            ImmTypeEnum.immJ.asUInt    -> imm_j
    ))
    
    ioGPR.src1 := IMEMio.inst_data(19, 15)
    ioGPR.src2 := IMEMio.inst_data(24, 20)
    ioGPR.rd   := IMEMio.inst_data(11, 7)
    ioGPR.wen  := decodeResult(GenWen)
}