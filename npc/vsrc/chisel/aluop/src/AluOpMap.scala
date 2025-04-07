package aluop

import chisel3._
import chisel3.util._

object AluOp extends ChiselEnum {
  val ADD, SUB, XOR, OR, AND, SLL, SLR, SRA, MUL, DIV, REM = Value
}

object AluOpMap {
  // 指令到ALU操作的映射表
  val AddSeq : Seq[String] = Seq(
    "auipc", "addi", "addiw",
    "lb", "lh", "lw", "ld", "lbu", "lhu", "lwu",
    "sb", "sh", "sw", "sd",
    "add", "addw",
    "fence", "ebreak", "jalr", "jal", "lui", "ecall"
  )
  val SubSeq : Seq[String] = Seq(
    "slti", "sltiu", "slt", "sltu",
    "sub", "subw",
    "beq", "bne", "blt", "bge", "bltu", "bgeu"
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
    "slli", "slliw", "sllw", "sll"
  )
  val SrlSeq : Seq[String] = Seq(
    "srli", "srl", "srliw", "srlw"
  )
  val SraSeq : Seq[String] = Seq(
    "srai", "sra", "sraiw", "sraw"
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

  // 转换函数
  def getAluOp(mnemonic: String): AluOp.Type = mnemonic match {
    case m if AddSeq.contains(m) => AluOp.ADD
    case m if SubSeq.contains(m) => AluOp.SUB
    case m if XorSeq.contains(m) => AluOp.XOR
    case m if OrSeq.contains(m)  => AluOp.OR
    case m if AndSeq.contains(m) => AluOp.AND
    case m if SllSeq.contains(m) => AluOp.SLL
    case m if SrlSeq.contains(m) => AluOp.SLR
    case m if SraSeq.contains(m) => AluOp.SRA
    case m if MulSeq.contains(m) => AluOp.MUL
    case m if DivSeq.contains(m) => AluOp.DIV
    case m if RemSeq.contains(m) => AluOp.REM
     case _ => throw new IllegalArgumentException(s"Unknown mnemonic: $mnemonic")
  }
}
