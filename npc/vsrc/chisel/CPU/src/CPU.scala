package cpu

import chisel3._

import gpr.GPR
import ifu.IFU
import imem.IMEM_d
import idu.IDU
import alu.ALU
import dmem.DMEM
import dmem.DMEM_d
import wbu.WBU

class CPU extends Module {

    val gpr = Module(new GPR())
    val ifu = Module(new IFU())
    val imem = Module(new IMEM_d())
    val idu = Module(new IDU())
    val alu = Module(new ALU())
    val dmem = Module(new DMEM_d())
    val wbu = Module(new WBU())

    imem.IFUio <> ifu.ioIMEM
    idu.IMEMio <> imem.ioIDU
    val data2 = Mux(idu.ioMUX.alud2slct, gpr.dataOutIO.data2, idu.ioMUX.imm)
    alu.IDUio <> idu.ioALU
    alu.MUXio.data2 := data2
    alu.GPRio.data1 := gpr.dataOutIO.data1
    dmem.ALUio <> alu.ioMEMWB
    dmem.GPRio.wdata := gpr.dataOutIO.data2
    val dataWB = Mux(idu.ioMUX.mpasslct, dmem.ioMUX.rdata, alu.ioMEMWB.res_addr)
    wbu.MUXio.dataWB := dataWB
    gpr.GPRio <> idu.ioGPR
    gpr.dataInIO <> wbu.ioGPR
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
        .emitSystemVerilogFile(new cpu.CPU(), args :+ "--target-dir" :+ "vsrc/", firtoolOptions)
    )
}

