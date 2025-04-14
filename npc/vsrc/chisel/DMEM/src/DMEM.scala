package dmem

import chisel3._
import chisel3.util.HasBlackBoxResource
import chisel3.util.HasBlackBoxPath
import chisel3.util.Fill
import chisel3.util.Cat
import chisel3.util.MuxLookup

import alu.IODMEMUX
import idu.IODMEM

class DMEMIO extends Bundle {
    val men   = Input(Bool())
    val mwen  = Input(Bool())
    val wmask = Input(UInt(8.W))
    val waddr = Input(UInt(64.W))
    val wdata = Input(UInt(64.W))
    val raddr = Input(UInt(64.W))
    val rdata = Output(UInt(64.W))
}

class DMEM extends BlackBox with HasBlackBoxPath {
    val io = IO(new DMEMIO)

    // Set the resource path for the Verilog file
    addPath("/home/miil/ysyx-workbench/npc/vsrc/chisel/DMEM/src/DMEM.v")
}

class DMEM_d extends Module {
    val ALUio = IO(Flipped(new alu.IODMEMUX))
    val IDUio = IO(Flipped(new idu.IODMEM))
    val GPRio = IO(new Bundle {
        val wdata = Input(UInt(64.W))
    })
    val ioMUX = IO(new Bundle {
        val rdata = Output(UInt(64.W))
    })

    val dmem = Module(new DMEM())
    dmem.io.wmask := IDUio.wmask
    dmem.io.waddr := ALUio.res_addr
    dmem.io.wdata := GPRio.wdata
    dmem.io.raddr := ALUio.res_addr
    dmem.io.men := IDUio.menslct
    dmem.io.mwen := IDUio.mwen
    val lbSext = Cat(Fill(56, dmem.io.rdata(7)), dmem.io.rdata(7,0))
    val lhSext = Cat(Fill(48, dmem.io.rdata(15)), dmem.io.rdata(15,0))
    val lwSext = Cat(Fill(32, dmem.io.rdata(31)), dmem.io.rdata(31,0))

    ioMUX.rdata := MuxLookup(IDUio.sextPos, dmem.io.rdata)(
        Seq(
            0.U -> lbSext,
            1.U -> lhSext,
            2.U -> lwSext,
            3.U -> dmem.io.rdata
        )
    )
}
