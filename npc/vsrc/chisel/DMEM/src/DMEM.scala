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
    addPath("/home/miil/ysyx/ysyx-workbench/npc/vsrc/chisel/DMEM/src/DMEM.v")
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
    val lbExt = Cat(Fill(56, Mux(IDUio.extSign, dmem.io.rdata(7 ), 0.U)), dmem.io.rdata(7 ,0))
    val lhExt = Cat(Fill(48, Mux(IDUio.extSign, dmem.io.rdata(15), 0.U)), dmem.io.rdata(15,0))
    val lwExt = Cat(Fill(32, Mux(IDUio.extSign, dmem.io.rdata(31), 0.U)), dmem.io.rdata(31,0))

    ioMUX.rdata := MuxLookup(IDUio.extPos, dmem.io.rdata)(
        Seq(
            0.U -> lbExt,
            1.U -> lhExt,
            2.U -> lwExt,
            3.U -> dmem.io.rdata
        )
    )
}
