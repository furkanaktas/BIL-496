package com.greenhouse.model

import java.sql.Timestamp

import slick.jdbc.PostgresProfile.api._
import slick.jdbc.PostgresProfile.api.{Tag => STag}


case class Greenhouse(id: Option[Long] = None,
                      gh_id: Int,
                      temperature: Double,
                      humidity: Double,
                      level: Int,
                      idate: Option[Timestamp])

object Tables {


  class Greenhouses(tag: STag) extends Table[Greenhouse](tag, "ghouses") {
    def id = column[Option[Long]]("id", O.PrimaryKey, O.AutoInc)

    def ghouseID = column[Int]("gh_id")

    def temperature = column[Double]("temperature")

    def humidity = column[Double]("humidity")

    def level = column[Int]("tlevel")

    def idate = column[Option[Timestamp]]("idate")


    def * =
      (id, ghouseID, temperature, humidity, level, idate) <> (Greenhouse.tupled, Greenhouse.unapply)
  }

  val greenHouses = TableQuery[Greenhouses]

}
