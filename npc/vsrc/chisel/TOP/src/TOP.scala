package top

import chisel3._

import ifu.IFU
import imem.IMEM
// import idu.IDU
// import alu.ALU
// import dmem.DMEM
// import wbu.WBU

class TOP extends Module {
    val io = IO(new Bundle {
        val inst_data = Output(UInt(32.W))
    })

    val ifu = Module(new IFU())
    val imem = Module(new IMEM())
    // val idu = Module(new IDU())
    // val alu = Module(new ALU())
    // val dmem = Module(new DMEM())
    // val wbu = Module(new WBU())

    imem.io.pc := ifu.ioIMEM.pc
    io.inst_data := imem.io.inst_data
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
        .emitSystemVerilogFile(new top.TOP(), args :+ "--target-dir" :+ "vsrc/", firtoolOptions)
    )
}

