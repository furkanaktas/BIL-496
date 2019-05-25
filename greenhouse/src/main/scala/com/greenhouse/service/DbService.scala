package com.greenhouse.service

import java.sql.Timestamp
import java.util.Calendar

import com.greenhouse.model._
import com.greenhouse.model.Tables._
import slick.jdbc.PostgresProfile.api._

import scala.concurrent.{ExecutionContext, Future}

class DbService(db: Database) {

  private def now = new Timestamp(Calendar.getInstance().getTimeInMillis)


  def getDataByGHId(ghID: Int)(implicit ec: ExecutionContext): Future[Seq[GHResponse]] = {
    db.run(greenHouses.filter(_.ghouseID === ghID).sortBy(_.id.desc).result).map(row =>
      row.map(e =>
        GHResponse(e.id.get, e.gh_id, e.temperature, e.humidity, e.level, e.idate.get)
      ))
  }

  def getLastData(ghID: Int)(implicit ec: ExecutionContext): Future[Option[GHResponse]] = {
    db.run(greenHouses.filter(_.ghouseID === ghID).sortBy(_.id.desc).result.headOption).map(row =>
      row.map(e =>
        GHResponse(e.id.get, e.gh_id, e.temperature, e.humidity, e.level, e.idate.get)
      ))
  }

  def getDataAll()(implicit ec: ExecutionContext): Future[Seq[GHResponse]] = {
    db.run(greenHouses.sortBy(_.id.desc).result).map(row =>
      row.map(e =>
        GHResponse(e.id.get, e.gh_id, e.temperature, e.humidity, e.level, e.idate.get)
      ))
  }

  def insertGHData(greenHouse: GHRequest): Future[Int] = {
    db.run(
      greenHouses += Greenhouse(None, greenHouse.ghID, greenHouse.temperature, greenHouse.humidity, greenHouse.level, Some(now))
    )
  }


  /*
    def updateSpecialPage(specialPage: SpecialPage)(implicit ec: ExecutionContext): Future[Int] = {
      val updateQuery = for {
        page <- greenHouses if page.id === specialPage.id.get
      } yield (page.pageName, page.url, page.header, page.headerText, page.paragraph)

      db.run(
        updateQuery.update((specialPage.pageName, specialPage.url, specialPage.header, specialPage.headerText, specialPage.paragraph))
      )
    }
  */
}
