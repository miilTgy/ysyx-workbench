package alu

import chisel3._

class ALU extends Module {
    val io = IO(new Bundle{
        val pc = Input(UInt(64.W))
        val aluop = Input(UInt(4.W))
        val data1 = Input(UInt(64.W))
        val data2 = Input(UInt(64.W))
        val result = Output(UInt(64.W))
    })

    io.result := io.data1 + io.data2
}