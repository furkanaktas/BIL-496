#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>


////   ------------------- DHT Sensor -----------------
#include "DHT.h"

#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

//// Water Tank Levels
#define level_1 5 
#define level_2 14
#define level_3 12
#define level_4 13
#define level_5 3

DHT dht(DHTPIN, DHTTYPE);
String result;

void setup() {
  delay(500); // to handle some error while waking up after sleeping (Possible Hardware Error)

  Serial.begin(115200);  // wifi

  char* ssid = "ESP12E";
  char* password =  "6101461ts";
  
  WiFi.begin(ssid, password);

  //  ---  DHT  ---
  pinMode(DHTPIN, INPUT_PULLUP);    // sets the digital pin 13 as output
  // Water Tank 
  pinMode(level_1, INPUT_PULLUP);    // 5   D1
  pinMode(level_2, INPUT_PULLUP);    // 14  D5
  pinMode(level_3, INPUT_PULLUP);    // 12  D6
  pinMode(level_4, INPUT_PULLUP);    // 13  D7
  pinMode(level_5, INPUT_PULLUP);    // 3   RX

  dht.begin();

  float temperature = 20.00;
  float humidity = 30.00;
  int tlevel = 0;
  int ghID =2; // green house id
  result = String("ghID="+String(ghID)+"&temp="+ String(temperature, 2)+ "&hum="+ String(humidity, 2)+ "&level=" + String(tlevel)+"\r\n");
  
  bool isOK = false;

  while(!isOK){
    // Reading humidity 
    humidity = dht.readHumidity();
    // Read temperature as Celsius (the default)
    temperature= dht.readTemperature();
  
    // Check if any reads failed and exit early (to try again).
    if (!isnan(temperature) && !isnan(humidity) ) {
      isOK=true;      
    }
  }
  Serial.println("");
  Serial.println(String(temperature));
  Serial.println(String(humidity));

  // Read Water Tank Levels
  int level5 = analogRead(level_5);
  int level4 = analogRead(level_4);
  int level3 = analogRead(level_3);
  int level2 = analogRead(level_2);
  int level1 = analogRead(level_1);


 /*
  Serial.println(F("\nbeyaz : "));

  while(1){
    Serial.println("L5: " + String(level5));  
    Serial.println("L4: " + String(level4));  
    Serial.println("L3: " + String(level3));  
    Serial.println("L2: " + String(level2));  
    Serial.println("L1: " + String(level1));  
    Serial.println();
    
    delay(2500);

    level5 = analogRead(level_5);
    level4 = analogRead(level_4);
    level3 = analogRead(level_3);
    level2 = analogRead(level_2);
    level1 = analogRead(level_1);
    
    }
  */

  // if 0, so water is on that level
  if(level5 == 0){
    tlevel =5;
    }
  else if (level4 == 0){
    tlevel =4;
    }
  else if (level3 == 0){
    tlevel =3;
    }
  else if (level2 == 0){
    tlevel =2;
    }
  else if (level1 == 0){
    tlevel =1;
    }
  else{
    tlevel =0;
    }

  // preapare sending message
  result = String("ghID="+String(ghID)+"&temp="+ String(temperature, 2)+ "&hum="+ String(humidity, 2)+ "&level=" + String(tlevel)+"\r\n");
  Serial.println(result);
  
  
  //ADC_MODE(ADC_VCC);          // vcc read mode
  //float vcc = ESP.getVcc();   //reading voltage

  WiFiClient client;
  char* host = "192.168.61.61";  // staticly set on server side.
  const uint16_t port = 8080;
  
  
  int sleepTime  = 20e6; // this is set short time, for demo
  int errorSleep = 10e6; // this is set short time, for demo
  int delayTime =100;
  unsigned long stuckTime = millis();

  while (WiFi.status() != WL_CONNECTED) {
    delay(delayTime);
    Serial.println("try to connect WiFi...");
    if (millis() - stuckTime >= 20000){ // for demp 20 sec, noramlly wait 3 min max to connect the wifi 
      ESP.deepSleep(errorSleep);  
    }
  }
  
  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());

  stuckTime = millis();
  while(!client.connect(host, port)) {      
    Serial.println("Connection to host failed");
    if (millis() - stuckTime >= 30000){ // for demo 30 second, normally 1 min
      ESP.deepSleep(errorSleep);  
    }
    delay(delayTime);
  }
  Serial.println("Connected to server successful!");  

  stuckTime = millis();
  while(WiFi.status() == WL_CONNECTED && client.connected() && !client.available() ){   // wait permission from server
    Serial.println("Waiting permission...");
    if (millis() - stuckTime >= 30000){ // for demp 30 sec, noramlly wait 3 min max to connect the wifi
      Serial.println("if Sleep...");
      client.stop();
      delay(1000);
      ESP.deepSleep(errorSleep);  
    }
    delay(delayTime);
  }
      

  if(WiFi.status() == WL_CONNECTED && client.connected()){
    Serial.println("msg waiting...");
    String line = client.readStringUntil('\0');
    Serial.println(String("received msg: "+line+"  "+ String(line.length())));

    int temp = line.toInt();

    if(temp !=0 ){
      sleepTime = temp*1e6;
      Serial.println(sleepTime);
      }
    
    delay(500);

    Serial.println(result);
    client.print(result);   // send result to server ( center wifi )
    
    Serial.println("Msg sent!!");

    stuckTime = millis();
    while(client.connected()){
      Serial.println("waiting until connection lost...");
      if (millis() - stuckTime >= 10000){ // for demp 10 sec, noramlly 30 sec
        client.stop();
        delay(1000);
        Serial.println("if Sleep...");
        ESP.deepSleep(sleepTime);  
      }
      delay(delayTime);  
    }  // wait until connection lost
  }  
  Serial.println("Sleep...");
  ESP.deepSleep(sleepTime);    // then deep sleep


}

void loop() {}
