package csr

import chisel3._
import chisel3.util.Mux1H

import idu.IOCSR
import csrop.CSROp
import ifu.IOIMEM

class WBUIO extends Bundle {
  val csrWB = Input(UInt(64.W))
}

class IOMUX extends Bundle {
  val csrRdata = Output(UInt(64.W))
  val dnpc = Output(UInt(64.W))
}

class CSRctl extends Module {
    val WBUio = IO(new WBUIO)
    val ioMUX = IO(new IOMUX)
    val IDUio = IO(Flipped(new idu.IOCSR))
    val IFUio = IO(Flipped(new ifu.IOIMEM))

    val CSRs = Module(new CSR)

    val isEcall = IDUio.csrop === csrop.CSROp.ECALL.asUInt
    val isMret = IDUio.csrop === csrop.CSROp.MRET.asUInt

    CSRs.CSRio.mstatusWen := ((IDUio.csrWaddr === CSRIndex.MSTATUS.asUInt) & IDUio.csrWenslct) | isEcall | isMret
    CSRs.CSRio.mtvecWen   := ((IDUio.csrWaddr === CSRIndex.MTVEC.asUInt) & IDUio.csrWenslct)
    CSRs.CSRio.mepcWen    := ((IDUio.csrWaddr === CSRIndex.MEPC.asUInt) & IDUio.csrWenslct) | isEcall
    CSRs.CSRio.mcauseWen  := ((IDUio.csrWaddr === CSRIndex.MCUASE.asUInt) & IDUio.csrWenslct) | isEcall

    val Adata = Mux1H(Seq(
        (IDUio.csrop === csrop.CSROp.WRITE.asUInt) -> WBUio.csrWB,
        (IDUio.csrop === csrop.CSROp.SET.asUInt) -> (ioMUX.csrRdata | WBUio.csrWB),
        (IDUio.csrop === csrop.CSROp.CLEAR.asUInt) -> (ioMUX.csrRdata & ~WBUio.csrWB),
    ))

    val MPIE = CSRs.CSRio.mstatusRdata(7)
    val MIE = CSRs.CSRio.mstatusRdata(3)

    // val mstatusEcall = (CSRs.CSRio.mstatusRdata & ~(1.U << 7)) |
    //                     (CSRs.CSRio.mstatusRdata(63, 8) ## MIE ## CSRs.CSRio.mstatusRdata(6, 0)) // MPIE = MIE
    val mstatusEcall = CSRs.CSRio.mstatusRdata

    // val mstatusMret = (CSRs.CSRio.mstatusRdata(63, 4) ## MPIE ## CSRs.CSRio.mstatusRdata(2, 0)) | // MIE = MPIE
    //                     (CSRs.CSRio.mstatusRdata(63, 8) ## 1.U(1.W) ## CSRs.CSRio.mstatusRdata(6, 0)) // MPIE = 1
    val mstatusMret = CSRs.CSRio.mstatusRdata
    dontTouch(mstatusEcall)
    dontTouch(mstatusMret)

    val mstatusEdata = Mux(isEcall, mstatusEcall, 0.U) | Mux(isMret, mstatusMret, 0.U)

    val mtvecEdata = CSRs.CSRio.mtvecRdata

    val mepcEcall = IFUio.pc
    val mepcEdata = mepcEcall

    val mcauseEcall = 11.U
    val mcauseEdata = mcauseEcall

    CSRs.CSRio.mstatusWdata := Mux(IDUio.csrWenslct, Adata, mstatusEdata)
    CSRs.CSRio.mtvecWdata   := Mux(IDUio.csrWenslct, Adata, mtvecEdata)
    CSRs.CSRio.mepcWdata    := Mux(IDUio.csrWenslct, Adata, mepcEdata)
    CSRs.CSRio.mcauseWdata  := Mux(IDUio.csrWenslct, Adata, mcauseEdata)

    ioMUX.csrRdata := Mux1H(Seq(
        (IDUio.csrRaddr === CSRIndex.MSTATUS.asUInt) -> CSRs.CSRio.mstatusRdata,
        (IDUio.csrRaddr === CSRIndex.MTVEC.asUInt) -> CSRs.CSRio.mtvecRdata,
        (IDUio.csrRaddr === CSRIndex.MEPC.asUInt) -> CSRs.CSRio.mepcRdata,
        (IDUio.csrRaddr === CSRIndex.MCUASE.asUInt) -> CSRs.CSRio.mcauseRdata,
    ))

    ioMUX.dnpc := Mux(isEcall, CSRs.CSRio.mtvecRdata, CSRs.CSRio.mepcRdata)
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
        .emitSystemVerilogFile(new CSRctl(), args :+ "--target-dir" :+ "vsrc/", firtoolOptions)
    )
}
