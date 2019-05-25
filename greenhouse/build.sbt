name := "greenhouse"

organization := "com.gh"

version := "1.12.0"

scalaVersion in ThisBuild := "2.12.6"

scalacOptions := Seq(
  "-deprecation",
  "-encoding", "UTF-8", // yes, this is 2 args
  "-feature",
  "-language:existentials",
  "-language:higherKinds",
  "-language:implicitConversions",
  "-unchecked",
  "-Xfatal-warnings",
  "-Xlint",
  "-Yno-adapted-args",
  "-Ywarn-dead-code",
  "-Ywarn-numeric-widen",
  "-Ywarn-value-discard",
  "-Xfuture",
  "-Ywarn-unused-import"
)

libraryDependencies ++= {
  val akkaHttpV = "10.1.1"
  val akkaV = "2.5.13"
  //val cassandraDriverV = "3.3.2"
  //val metricsV = "3.5.9"
  val prometheusSimpleclientV = "0.3.0"
  val scalaLoggingV = "3.7.2"
  val logbackV = "1.2.3"
  val logbackEncoderV = "4.11"
  //val scalaTestV = "3.0.5"
  //val redisScalaV = "1.8.0"
  val tototoshiV = "1.3.5"
  val guavaV = "16.0.1"
  val jsr305V = "3.0.1"
  //val mockitoV = "1.8.4"
  val dispatchV = "0.13.3"
  val scalaCacheV = "0.9.4"
  val slickV = "3.2.1"
  val postgresV = "42.1.4"
  val jxmCollectorV = "0.10"


  Seq(
    "com.typesafe.akka" %% "akka-http-spray-json" % akkaHttpV,
    "com.typesafe.akka" %% "akka-stream" % akkaV,
    "com.typesafe.akka" %% "akka-slf4j" % akkaV,
    //"com.datastax.cassandra" % "cassandra-driver-core" % cassandraDriverV,
    //"nl.grons" %% "metrics-scala" % metricsV,
    "io.prometheus.jmx" % "collector" % jxmCollectorV,
    //"io.prometheus" % "simpleclient_dropwizard" % prometheusSimpleclientV,
    //"io.prometheus" % "simpleclient_common" % prometheusSimpleclientV,
    "io.prometheus" % "simpleclient_hotspot" % prometheusSimpleclientV,
    "com.typesafe.scala-logging" %% "scala-logging" % scalaLoggingV,
    "ch.qos.logback" % "logback-core" % logbackV,
    "ch.qos.logback" % "logback-classic" % logbackV,
    "net.logstash.logback" % "logstash-logback-encoder" % logbackEncoderV,
    //"com.github.etaty" %% "rediscala" % redisScalaV,
    "com.google.guava" % "guava" % guavaV,
    "com.google.code.findbugs" % "jsr305" % jsr305V,
    "com.github.tototoshi" %% "scala-csv" % tototoshiV,
    "com.github.cb372" %% "scalacache-caffeine" % scalaCacheV,
    "com.typesafe.slick" %% "slick" % slickV,
    "com.typesafe.slick" %% "slick-hikaricp" % slickV,
    "org.postgresql" % "postgresql" % postgresV,
    //"org.scalatest" %% "scalatest" % scalaTestV % Test,
    //"org.mockito" % "mockito-all" % mockitoV % Test,
    "com.typesafe.akka" %% "akka-http-testkit" % akkaHttpV % Test,
    "net.databinder.dispatch" %% "dispatch-core" % dispatchV
  )
}


