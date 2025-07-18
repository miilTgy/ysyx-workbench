package csrop

import chisel3._
import chisel3.util._

object CSROp extends ChiselEnum {
    val NONE, WRITE, SET, CLEAR, ECALL, MRET = Value
}

object CSROpMap {
    val WriteSeq: Seq[String] = Seq(
        "csrrw", "csrrwi"
    )
    val SetSeq: Seq[String] = Seq(
        "csrrs", "csrrsi"
    )
    val ClearSeq: Seq[String] = Seq(
        "csrrc", "csrrci"
    )
    val EcallSeq: Seq[String] = Seq(
        "ecall"
    )
    val MretSeq: Seq[String] = Seq(
        "mret"
    )

    def getCSROp(mnemonic: String): CSROp.Type = mnemonic match {
        case m if WriteSeq.contains(m) => CSROp.WRITE
        case m if SetSeq.contains(m)   => CSROp.SET
        case m if ClearSeq.contains(m) => CSROp.CLEAR
        case m if EcallSeq.contains(m) => CSROp.ECALL
        case m if MretSeq.contains(m)  => CSROp.MRET
        case _                         => CSROp.NONE
    }
}