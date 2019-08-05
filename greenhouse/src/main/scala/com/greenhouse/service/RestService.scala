package com.greenhouse.service


import akka.http.scaladsl.marshallers.sprayjson.SprayJsonSupport
import akka.http.scaladsl.server.Directives._
import akka.http.scaladsl.server._
import com.greenhouse.model.Protocol._
import com.greenhouse.model._
import scalacache.ScalaCache
import scalacache.serialization.InMemoryRepr

import scala.concurrent.{ExecutionContext}

class RestService(ghService: GreenhouseService)
                 (implicit ec: ExecutionContext, implicit val scalaCache: ScalaCache[InMemoryRepr])
  extends SprayJsonSupport {


  // greenhouse CRUD endpoints
  private val endpoints = path("") {
    get {
      parameters('ghID.as[Int]) { ghID =>
        complete(ghService.getDataByGHId(ghID))
      }
    } ~ post {
      entity(as[GHRequest]) { data =>
        println("posta girdi !!")
        complete(ghService.addInfo(data).map(_.toString))
      }
    } ~ put {
      parameters(('ghID.as[Int], 'temp.as[Double], 'hum.as[Double], 'level.as[Int])) { (ghID, temp, hum, level) =>
        complete(ghService.addInfo(ghID, temp, hum, level).map(_.toString))
      }
    } ~ post {
      parameters(('ghID.as[Int], 'temp.as[Double], 'hum.as[Double], 'level.as[Int])) { (ghID, temp, hum, level) =>
        complete(ghService.addInfo(ghID, temp, hum, level).map(_.toString))
      }
    } ~ get {
      parameters('ghID2.as[Int]) { ghID =>
        complete(ghService.getLastData(ghID))
      }
    } ~ get {
      complete(ghService.getAllData())
    }
    /*~ put {
      entity(as[WikiBoxForUpdate]) { data =>
        complete(wikiboxService.updateWikiboxContent(data).map(_.toString))
      }
    }  ~ delete {
      entity(as[WikiBoxForDelete]) { data =>
        complete(wikiboxService.deleteWikibox(data).map(_.toString))
      }
    }*/
  } ~ path("hello") {
    get {
      complete("Hello World")
    }
  }


  val routes = (Route.seal(endpoints))
}
