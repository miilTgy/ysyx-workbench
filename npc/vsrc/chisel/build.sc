import os.copy.over
// import Mill dependency
import mill._
import mill.define.Sources
import mill.modules.Util
import mill.scalalib.scalafmt.ScalafmtModule
import mill.scalalib.TestModule.ScalaTest
import scalalib._
// support BSP
import mill.bsp._

trait chiselModule extends ScalaModule with ScalafmtModule { m =>
  override def scalaVersion = "2.13.15"
  override def scalacOptions = Seq(
    "-language:reflectiveCalls",
    "-deprecation",
    "-feature",
    "-Xcheckinit",
  )
  override def ivyDeps = Agg(
    ivy"org.chipsalliance::chisel:6.6.0",
  )
  override def scalacPluginIvyDeps = Agg(
    ivy"org.chipsalliance:::chisel-plugin:6.6.0",
  )
  object test extends ScalaTests with TestModule.ScalaTest with ScalafmtModule {
    override def ivyDeps = m.ivyDeps() ++ Agg(
      ivy"org.scalatest::scalatest::3.2.19",
      // for formal flow in future
      ivy"edu.berkeley.cs::chiseltest:6.0.0"
    )
  }
  def repositoriesTask = T.task {
    Seq(
      coursier.MavenRepository("https://repo.scala-sbt.org/scalasbt/maven-releases"),
      coursier.MavenRepository("https://oss.sonatype.org/content/repositories/releases"),
      coursier.MavenRepository("https://oss.sonatype.org/content/repositories/snapshots")
    ) ++ super.repositoriesTask()
  }
  println("super.millSourcePath: " + super.millSourcePath)
  def sources = T.sources {
    super.sources() ++ Seq(
      PathRef(super.millSourcePath / ".." / "rvdecoderdb" / "rvdecoderdb")
    )
  }
}

object GPR extends chiselModule {}
object IFU extends chiselModule {}
object IMEM extends chiselModule {
  override def moduleDeps = Seq(IFU)
}
object aluop extends chiselModule {}
object IDU extends chiselModule {
  override def moduleDeps = Seq(GPR, aluop, IMEM)
}
object ALU extends chiselModule {
  override def moduleDeps = Seq(aluop, IDU)
}
object LSU extends chiselModule {}
object DMEM extends chiselModule {
  override def moduleDeps = Seq(ALU, IDU)
}
object WBU extends chiselModule {}
object CPU extends chiselModule {
  override def moduleDeps = Seq(GPR, IFU, IMEM, IDU, aluop, ALU, LSU, DMEM, WBU)
}