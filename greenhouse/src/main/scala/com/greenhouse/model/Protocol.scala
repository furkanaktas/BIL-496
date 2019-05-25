package com.greenhouse.model

import java.sql.Timestamp

import spray.json._

object Protocol extends DefaultJsonProtocol {

  implicit val timestampJsonFormat: RootJsonFormat[Timestamp] = new RootJsonFormat[Timestamp] {
    override def read(json: JsValue): Timestamp = json match {
      case JsNumber(millis) => new Timestamp(millis.toLong)
      case _ => throw DeserializationException(s"expected JsNumber, got $json")
    }
    override def write(obj: Timestamp): JsValue = JsNumber(obj.getTime)
  }

  implicit val greenHouseRequestFormat = jsonFormat4(GHRequest)
  implicit val greenHouseResponseFormat = jsonFormat6(GHResponse)

}
