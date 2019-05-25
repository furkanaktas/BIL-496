package com.greenhouse.config

import com.typesafe.config.ConfigFactory

object AppConfig {

  val config = ConfigFactory.load()

  val serverHost = config.getString("server.host")
  val serverPort = config.getInt("server.port")

  val cacheMaxSize = config.getLong("cache.maximumSize")
  val cacheExpireSeconds = config.getInt("cache.expireSeconds")


}
