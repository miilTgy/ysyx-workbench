package cpu

import chisel3._
import chisel3.dontTouch

import gpr.GPR
import ifu.IFU
import imem.IMEM_d
import idu.IDU
import alu.ALU
import dmem.DMEM
import dmem.DMEM_d
import wbu.WBU
import csr.CSRctl

class CPU extends Module {
    val io = IO(new Bundle {
        val pc = Output(UInt(64.W))
    })

    val gpr = Module(new GPR())
    val ifu = Module(new IFU())
    val imem = Module(new IMEM_d())
    val idu = Module(new IDU())
    val alu = Module(new ALU())
    val dmem = Module(new DMEM_d())
    val wbu = Module(new WBU())
    val csr = Module(new CSRctl())
    
    io.pc := ifu.ioIMEM.pc

    imem.IFUio <> ifu.ioIMEM
    idu.IFUio <> ifu.ioIMEM
    idu.IMEMio <> imem.ioIDU
    
    val data1 = Mux(idu.ioMUX.alud1slct, idu.ioPCs.pc, gpr.dataOutio.data1)
    alu.MUXio.data1 := data1
    val pcInc = Mux(idu.ioMUX.pcInc4slct, 4.U(64.W), idu.ioMUX.imm)
    val data2 = Mux(idu.ioMUX.alud2slct, gpr.dataOutio.data2, pcInc)
    alu.MUXio.data2 := data2
    
    alu.IDUio <> idu.ioALU
    dmem.ALUio <> alu.ioDMEMUX
    dmem.IDUio <> idu.ioDMEM

    dmem.GPRio.wdata := gpr.dataOutio.data2
    val dataWB_1 = Mux(idu.ioDMEM.menslct, dmem.ioMUX.rdata, alu.ioDMEMUX.res_addr)
    val dataWB = Mux(idu.ioCSR.csrWenslct, csr.ioMUX.csrRdata, dataWB_1)
    wbu.MUXio.dataWB := dataWB

    gpr.GPRio <> idu.ioGPR
    gpr.dataInio <> wbu.ioGPR

    csr.IDUio <> idu.ioCSR
    csr.WBUio.csrWB := alu.ioDMEMUX.res_addr
    csr.IFUio <> ifu.ioIMEM

    val jmpslct = idu.ioJMP.jmpslct & alu.ioJMP.jmpslct
    val snpc = ifu.ioIMEM.pc + 4.U
    val pcsrc = Mux(idu.ioMUX.pcsrcslct, gpr.dataOutio.data1, ifu.ioIMEM.pc)
    dontTouch(snpc)
    val dnpc = idu.ioMUX.imm + pcsrc
    dontTouch(dnpc)
    val nextpc = Mux(jmpslct, dnpc, snpc)
    ifu.MUXio.pcNext := Mux(idu.ioMUX.Expcslct, csr.ioMUX.dnpc, nextpc)
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
