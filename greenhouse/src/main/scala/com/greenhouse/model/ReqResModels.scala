package com.greenhouse.model

import java.sql.Timestamp


case class GHRequest(ghID: Int,
                     temperature: Double,
                     humidity: Double,
                     level: Int)

case class GHResponse(id: Long,
                      ghId: Int,
                      temperature: Double,
                      humidity: Double,
                      level: Int,
                      idate: Timestamp)
