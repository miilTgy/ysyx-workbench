package csr

import chisel3._
import chisel3.util.Mux1H

import idu.IOCSR

class CSRctlIO extends Bundle {
    val csrRdata = Output(UInt(64.W))
    val csrWdata = Input(UInt(64.W))
}

class CSRctl extends Module {
    val ioCSRctl = IO(new CSRctlIO)
    val IDUio = IO(Flipped(new IOCSR))

    val CSRs = Module(new CSR)

    CSRs.CSRio.mstatusWen := Mux(IDUio.csrWaddr === CSRIndex.MSTATUS.asUInt, IDUio.csrWenslct, false.B)
    CSRs.CSRio.mtvecWen   := Mux(IDUio.csrWaddr === CSRIndex.MTVEC.asUInt, IDUio.csrWenslct, false.B)
    CSRs.CSRio.mepcWen    := Mux(IDUio.csrWaddr === CSRIndex.MEPC.asUInt, IDUio.csrWenslct, false.B)
    CSRs.CSRio.mcauseWen  := Mux(IDUio.csrWaddr === CSRIndex.MCUASE.asUInt, IDUio.csrWenslct, false.B)

    CSRs.CSRio.mstatusWdata := ioCSRctl.csrWdata
    CSRs.CSRio.mtvecWdata   := ioCSRctl.csrWdata
    CSRs.CSRio.mepcWdata    := ioCSRctl.csrWdata
    CSRs.CSRio.mcauseWdata  := ioCSRctl.csrWdata

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
