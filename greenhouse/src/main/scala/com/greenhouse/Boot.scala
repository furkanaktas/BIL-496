package com.greenhouse


import akka.actor.ActorSystem
import akka.http.scaladsl.Http
import akka.stream.ActorMaterializer
import com.greenhouse.config.AppConfig._
import com.greenhouse.service._
import com.github.benmanes.caffeine.cache.Caffeine
import com.typesafe.scalalogging.StrictLogging
import io.prometheus.client.CollectorRegistry
import io.prometheus.client.hotspot.DefaultExports
import io.prometheus.jmx.JmxCollector
import scalacache.ScalaCache
import scalacache.caffeine.CaffeineCache
import scalacache.serialization.InMemoryRepr
import slick.jdbc.PostgresProfile.api.Database

object Boot extends App with StrictLogging {
  implicit val system = ActorSystem()
  implicit val mat = ActorMaterializer()

  CollectorRegistry.defaultRegistry.register(new JmxCollector("""whitelistObjectNames: ["slick:*"]"""))
  DefaultExports.initialize()


  val dbClient = Database.forConfig("db")

  val cacheConfig = CaffeineCache(Caffeine.newBuilder().maximumSize(cacheMaxSize).build[String, Object])
  implicit val scalaCache: ScalaCache[InMemoryRepr] = ScalaCache(cacheConfig)

  import system.dispatcher

  val dbService = new DbService(dbClient)

  val ghService = new GreenhouseService(dbService)

  val restService = new RestService(ghService)

  Http().bindAndHandle(restService.routes, serverHost, serverPort)
    .map(_ => logger.info(s"Server started at port $serverPort"))

}
