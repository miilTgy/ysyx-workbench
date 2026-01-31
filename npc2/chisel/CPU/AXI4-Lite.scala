package axi4lite

import chisel3._
import chisel3.dontTouch

class AXI4LiteMaster extends Bundle {
    val AR = Output(UInt(32.W))
    val R  = Input(UInt(64.W))
    val AW = Output(UInt(32.W))
    val W  = Output(UInt(64.W))
    val B  = Input(Bool())
}