package csr

import chisel3._
import chisel3.util.Mux1H

class CSRIO extends Bundle {
    val mstatusWen = Input(Bool())
    val mstatusWdata = Input(UInt(64.W))
    val mstatusRdata = Output(UInt(64.W))
    val mtvecWen = Input(Bool())
    val mtvecWdata = Input(UInt(64.W))
    val mtvecRdata = Output(UInt(64.W))
    val mepcWen = Input(Bool())
    val mepcWdata = Input(UInt(64.W))
    val mepcRdata = Output(UInt(64.W))
    val mcauseWen = Input(Bool())
    val mcauseWdata = Input(UInt(64.W))
    val mcauseRdata = Output(UInt(64.W))
}

class CSR extends Module {
    val CSRio = IO(new CSRIO)

    val mstatus = RegInit(0.U(64.W))
    val mtvec = Reg(UInt(64.W))
    val mepc = Reg(UInt(64.W))
    val mcause = Reg(UInt(64.W))

    mstatus := Mux(CSRio.mstatusWen, CSRio.mstatusWdata, mstatus)
    mtvec := Mux(CSRio.mtvecWen, CSRio.mtvecWdata, mtvec)
    mepc := Mux(CSRio.mepcWen, CSRio.mepcWdata, mepc)
    mcause := Mux(CSRio.mcauseWen, CSRio.mcauseWdata, mcause)

    CSRio.mstatusRdata := mstatus
    CSRio.mtvecRdata := mtvec
    CSRio.mepcRdata := mepc
    CSRio.mcauseRdata := mcause
}

object csrMain extends App {
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
        .emitSystemVerilogFile(new CSR(), args :+ "--target-dir" :+ "vsrc/", firtoolOptions)
    )
}
