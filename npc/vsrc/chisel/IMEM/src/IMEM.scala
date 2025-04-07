package imem

import chisel3._
import chisel3.util.HasBlackBoxResource

class IMEM extends BlackBox with HasBlackBoxResource {
  val io = IO(new Bundle {
    val pc = Input(UInt(64.W))
    val inst = Output(UInt(32.W))
  })

  // Set the resource path for the Verilog file
  addResource("/vsrc/chisel/IMEM/src/ysyx_25010001_IMEM.v")
}