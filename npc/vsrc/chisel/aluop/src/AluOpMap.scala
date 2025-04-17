package aluop

import chisel3._
import chisel3.util._

object AluOp extends ChiselEnum {
  val ADDSUB, UNSUB, XOR, OR, AND,
  SLL, SLR, SRA, MUL, DIV, REM,
  BEQ, BNE, BLT, BGE,
  SLLW, SRLW, SRAW, D2PASS = Value
}

object AluOpMap {
  // 指令到ALU操作的映射表
  val AddSeq : Seq[String] = Seq(
    "auipc", "addi", "addiw",
    "lb", "lh", "lw", "ld", "lbu", "lhu", "lwu",
    "sb", "sh", "sw", "sd",
    "add", "addw",
    "fence", "ebreak", "jalr", "jal", "ecall"
  )
  val SubSeq : Seq[String] = Seq(
    "slti", "slt",
    "sub", "subw"
  )
  val unSubSeq : Seq[String] = Seq(
    "sltiu", "sltu",
  )
  val XorSeq : Seq[String] = Seq(
    "xori", "xor"
  )
  val OrSeq : Seq[String] = Seq(
    "ori", "or"
  )
  val AndSeq : Seq[String] = Seq(
    "andi", "and"
  )
  val SllSeq : Seq[String] = Seq(
    "slli", "sll"
  )
  val SrlSeq : Seq[String] = Seq(
    "srli", "srl"
  )
  val SraSeq : Seq[String] = Seq(
    "srai", "sra"
  )
  val MulSeq : Seq[String] = Seq(
    "mul", "mulh", "mulhsu", "mulhu", "mulw"
  )
  val DivSeq : Seq[String] = Seq(
    "div", "divu", "divw", "divuw"
  )
  val RemSeq : Seq[String] = Seq(
    "rem", "remu", "remw", "remuw"
  )
  val BeqSeq : Seq[String] = Seq(
    "beq"
  )
  val BneSeq : Seq[String] = Seq(
    "bne"
  )
  val BltSeq : Seq[String] = Seq(
    "blt", "bltu"
  )
  val BgeSeq : Seq[String] = Seq(
    "bge", "bgeu"
  )
  val SllwSeq : Seq[String] = Seq(
    "slliw", "sllw"
  )
  val SrlwSeq : Seq[String] = Seq(
    "srliw", "srlw"
  )
  val SrawSeq : Seq[String] = Seq(
    "sraiw", "sraw"
  )
  val D2PassSeq : Seq[String] = Seq(
    "lui"
  )

  // 转换函数
  def getAluOp(mnemonic: String): AluOp.Type = mnemonic match {
    case m if AddSeq.contains(m) => AluOp.ADDSUB
    case m if SubSeq.contains(m) => AluOp.ADDSUB
    case m if unSubSeq.contains(m) => AluOp.UNSUB
    case m if XorSeq.contains(m) => AluOp.XOR
    case m if OrSeq.contains(m)  => AluOp.OR
    case m if AndSeq.contains(m) => AluOp.AND
    case m if SllSeq.contains(m) => AluOp.SLL
    case m if SrlSeq.contains(m) => AluOp.SLR
    case m if SraSeq.contains(m) => AluOp.SRA
    case m if MulSeq.contains(m) => AluOp.MUL
    case m if DivSeq.contains(m) => AluOp.DIV
    case m if RemSeq.contains(m) => AluOp.REM
    case m if BeqSeq.contains(m) => AluOp.BEQ
    case m if BneSeq.contains(m) => AluOp.BNE
    case m if BltSeq.contains(m) => AluOp.BLT
    case m if BgeSeq.contains(m) => AluOp.BGE
    case m if SllwSeq.contains(m) => AluOp.SLLW
    case m if SrlwSeq.contains(m) => AluOp.SRLW
    case m if SrawSeq.contains(m) => AluOp.SRAW
    case m if D2PassSeq.contains(m) => AluOp.D2PASS
    case _ => throw new IllegalArgumentException(s"Unknown mnemonic: $mnemonic")
  }
}