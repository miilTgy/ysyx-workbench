package csr

import chisel3._
import chisel3.util.Mux1H

import idu.IOCSR
import csrop.CSROp

class WBUIO extends Bundle {
  val csrWB = Input(UInt(64.W))
}

class IOMUX extends Bundle {
  val csrRdata = Output(UInt(64.W))
}

class CSRctl extends Module {
    val WBUio = IO(new WBUIO)
    val ioMUX = IO(new IOMUX)
    val IDUio = IO(Flipped(new IOCSR))

    val CSRs = Module(new CSR)

    CSRs.CSRio.mstatusWen := (IDUio.csrWaddr === CSRIndex.MSTATUS.asUInt) & IDUio.csrWen
    CSRs.CSRio.mtvecWen   := (IDUio.csrWaddr === CSRIndex.MTVEC.asUInt) & IDUio.csrWen
    CSRs.CSRio.mepcWen    := (IDUio.csrWaddr === CSRIndex.MEPC.asUInt) & IDUio.csrWen
    CSRs.CSRio.mcauseWen  := (IDUio.csrWaddr === CSRIndex.MCUASE.asUInt) & IDUio.csrWen

    val Wdata = Mux1H(Seq(
        (IDUio.csrop === csrop.CSROp.WRITE.asUInt) -> WBUio.csrWB,
        (IDUio.csrop === csrop.CSROp.SET.asUInt) -> (ioMUX.csrRdata | WBUio.csrWB),
        (IDUio.csrop === csrop.CSROp.CLEAR.asUInt) -> (ioMUX.csrRdata & ~WBUio.csrWB),
    ))

    CSRs.CSRio.mstatusWdata := Wdata
    CSRs.CSRio.mtvecWdata   := Wdata
    CSRs.CSRio.mepcWdata    := Wdata
    CSRs.CSRio.mcauseWdata  := Wdata

    ioCSRctl.csrRdata := Mux1H(Seq(
        (IDUio.csrRaddr === CSRIndex.MSTATUS.asUInt) -> CSRs.CSRio.mstatusRdata,
        (IDUio.csrRaddr === CSRIndex.MTVEC.asUInt) -> CSRs.CSRio.mtvecRdata,
        (IDUio.csrRaddr === CSRIndex.MEPC.asUInt) -> CSRs.CSRio.mepcRdata,
        (IDUio.csrRaddr === CSRIndex.MCUASE.asUInt) -> CSRs.CSRio.mcauseRdata,
    ))
}

object Main extends App {
    val firtoolOptions = Array(
    "--lowering-options=" + List(
      // make yosys happy
      // see https://github.com/llvm/circt/blob/main/docs/VerilogGeneration.md
      "disallowLocalVariables",
      "disallowPackedArrays",
      "locationInfoStyle=wrapInAtSquareBracket"
    ).reduce(_ + "," + _)
    )
    print("args: ")
    for (i <- args) {
      print(i + " ")
    }
    println()
    println(
      circt.stage.ChiselStage
        .emitSystemVerilogFile(new CSRctl(), args :+ "--target-dir" :+ "vsrc/", firtoolOptions)
    )
}
