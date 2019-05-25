package com.greenhouse.service


import com.greenhouse.model.{GHRequest, GHResponse}

import scala.concurrent.{ExecutionContext, Future}

class GreenhouseService(dbService: DbService) {

  def getDataByGHId(ghID: Int)(implicit ec: ExecutionContext): Future[Seq[GHResponse]] = {
    dbService.getDataByGHId(ghID)
  }

  def getLastData(ghID: Int)(implicit ec: ExecutionContext): Future[Option[GHResponse]] = {
    dbService.getLastData(ghID)
  }

  def getAllData()(implicit ec: ExecutionContext): Future[Seq[GHResponse]] = {
    dbService.getDataAll()
  }

  def addInfo(ghID: Int, temperature: Double, humidity: Double, level: Int ): Future[Int] = {
    dbService.insertGHData(GHRequest(ghID, temperature, humidity, level))
  }

  def addInfo(data: GHRequest): Future[Int] = {
    dbService.insertGHData(data)
  }
}
