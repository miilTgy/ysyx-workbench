package csr

import chisel3._

object CSRIndex extends ChiselEnum {
    val MSTATUS = Value(0x300.U);
    val MTVEC = Value(0x305.U);
    val MEPC = Value(0x341.U);
    val MCUASE = Value(0x342.U);
}
