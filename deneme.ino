// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor


// --------------------- Wifi Connection -----------------

#include <Arduino.h>


#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>


#include <WiFiClient.h>

//#include <WiFi.h>

//ESP8266WiFiMulti WiFiMulti;


////   ------------------- DHT Sensor -----------------
#include "DHT.h"

#define DHTPIN 4     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.
//#define FLMPIN 2 
// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.

DHT dht(DHTPIN, DHTTYPE);

String result;

void setup() {
  //Serial.begin(9600);
  //Serial.println(F("DHT11 test!"));
  delay(500);

  Serial.begin(115200);  // wifi

//  char* ssid = "Aktas";
//  char* password =  "GireBiLirseN61";

  char* ssid = "ESP12E";
  char* password =  "6101461ts";
  WiFi.begin(ssid, password);


  //  ---  DHT  ---
  //pinMode(FLMPIN, INPUT_PULLUP);
  pinMode(DHTPIN, INPUT_PULLUP);    // sets the digital pin 13 as output
  //pinMode(5, INPUT_PULLUP);    // sets the digital pin 13 as output  
  //digitalWrite(DHTPIN, LOW); // sets the digital pin 13 on
  dht.begin();

  float temperature = 0.00;
  float humidity = 0.00;
  int tlevel = 0;
  //result = String("ghID=0&temp="+ String(temperature, 2)+ "&hum="+ String(humidity, 2)+ "&level=" + String(tlevel)+"\r\n");
  

  bool isOK = false;

  while(!isOK){
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    humidity = dht.readHumidity();
    // Read temperature as Celsius (the default)
    temperature= dht.readTemperature();
  
    // Check if any reads failed and exit early (to try again).
    if (!isnan(temperature) && !isnan(humidity) ) {
      isOK=true;      
    }
  }
  Serial.println(String(temperature));
  Serial.println(String(humidity));
  result = String("ghID=0&temp="+ String(temperature, 2)+ "&hum="+ String(humidity, 2)+ "&level=" + String(tlevel)+"\r\n");
  
 
  /*
    Serial.println(F("\nbeyaz : "));
    int w = analogRead(5);
    if(w >= 750)
      w = 0;
    else
      w = 1;  
    Serial.println(w);

    Serial.println(F("siyah : "));
    int b = analogRead(DHTPIN);
    if(b >= 750)
      b = 0;
    else
      b = 1;
    Serial.println(b);

*/    

  //ADC_MODE(ADC_VCC);  // vcc read mode
  //float vcc = ESP.getVcc();     //voltaj ölçmek için

  WiFiClient client;
  char* host = "192.168.61.61";
  const uint16_t port = 8080;
  
  int num =0;
  int sleepTime = 40e6;
  int delayTime =100;
  unsigned long stuckTime = millis();

  while (WiFi.status() != WL_CONNECTED) {
    delay(delayTime);
    Serial.println("try to connect WiFi...");
    //if (millis() - stuckTime >= 180000){
    if (millis() - stuckTime >= 10000){
      //digitalWrite(DHTPIN, LOW);
      //delay(500);
      ESP.deepSleep(sleepTime);  
    }
    //Serial.println(String("result size : " + String(result.length()))); // size 35
    //Serial.println(result[35]);
    /*
    if(result[34] == '\n')
      Serial.println("null dahil değil , \n");
      
    if(result[35] == '\0')
      Serial.println("null dahil değil size,  36.");
      */
  }
  
  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());

  stuckTime = millis();
  while(!client.connect(host, port)) {      
    Serial.println("Connection to host failed");
    if (millis() - stuckTime >= 60000){
      Serial.println("if Sleep...");
      ESP.deepSleep(sleepTime);  
    }
    delay(delayTime);
  }
  Serial.println("Connected to server successful!");
  

  stuckTime = millis();
  while(WiFi.status() == WL_CONNECTED && client.connected() && !client.available() ){   // wait permission from server
    Serial.println("Waiting permission...");
    if (millis() - stuckTime >= 180000){
      Serial.println("if Sleep...");
      client.stop();
      delay(1000);
      ESP.deepSleep(sleepTime);  
    }
    delay(delayTime);
  }
      

  if(WiFi.status() == WL_CONNECTED && client.connected()){
    Serial.println("msg waiting...");
    String line = client.readStringUntil('\0');
    Serial.println(String("received msg: "+line+"  "+ String(line.length())));

    if(line[0] == '0')
    {
      Serial.println("0 if'e girdi... !");
      sleepTime = 40e6;
    }
    else  
    {
      sleepTime = 20e6;
    }
    
    delay(500);

    //ghID=0&temp=21.75&hum=32.63&level=3
    //String result = "2Send from client\r\n";

    //ghID=0&temp=21.75&hum=32.63&level=3
    
    Serial.println(result);
    client.print(result);  
    
    Serial.println("Msg sent!!");

    stuckTime = millis();
    while(client.connected()){
      Serial.println("waiting until connection lost...");
      if (millis() - stuckTime >= 30000){
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
