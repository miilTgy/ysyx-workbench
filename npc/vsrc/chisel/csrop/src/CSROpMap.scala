package csrop

import chisel3._
import chisel3.util._

object CSROp extends ChiselEnum {
    val NONE  = Value("b000".U)
    val WRITE = Value("b001".U)
    val SET   = Value("b010".U)
    val CLEAR = Value("b011".U)
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

    def getCSROp(mnemonic: String): CSROp.Type = mnemonic match {
        case m if WriteSeq.contains(m) => CSROp.WRITE
        case m if SetSeq.contains(m)   => CSROp.SET
        case m if ClearSeq.contains(m) => CSROp.CLEAR
        case _                         => CSROp.NONE
    }
}