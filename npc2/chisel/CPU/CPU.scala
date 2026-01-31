package cpu

import chisel3._
import chisel3.dontTouch

import axi4lite._

class CPU extends Module {
    val BUS = IO(new AXI4LiteMaster)
    BUS.AR := DontCare
    BUS.AW := DontCare
    BUS.W  := DontCare

    val ifu = Module(new IFU)
    val idu = Module(new IDU)
    val gpr = Module(new GPR)
    val csr = Module(new CSR)
    val alu = Module(new ALU)
    val lsu = Module(new LSU)
    val wbu = Module(new WBU)
}
