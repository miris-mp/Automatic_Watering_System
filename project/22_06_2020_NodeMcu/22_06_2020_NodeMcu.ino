#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
// Arduino Json version is the 5 and not the 6 because for use some library we have done the downgrade
#include <ArduinoJson.h>
#include <Wire.h>
//#include "CTBot.h"

// wifi access
char* ssid = "SSID";
char* password = "Password";

char* serverName = "http://miris.pythonanywhere.com/postjson";

//The udp library class
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
String dateAndTime = "";
int months[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
String day = "";
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String nameDay;
int hour = 0;
int minute = 0;
int seconds = 0;
int monthDay = 0;
String currentMonthName = "";
int currentYear = 0;

//moisture variable
// we have to take the values of the sensor for the calculation it's needed the airValue and the WaterValue as fix parameter
const int AirValue = 620;   //you need to replace this value with Value_1
const int WaterValue = 270;

// the optimal water value is checked with the percentage one
const int optimalWaterValue = 80;

/*
  const int dry = 520;
  const int wet = 270;
*/
int soilMoisturePercentage = 0;

const int SLAVE_ADDRESS = 42;

// slave didn't response
bool resp = false;
// various commands we might send
enum {
  CMD_ID = 1,
  CMD_READ_A0  = 2,
  CMD_READ_D2 = 3,
  CMD_ON = 4,
  CMD_OFF = 5
};

void setup ()
{
  Serial.begin(9600);
  Wire.begin(D1, D2);

  sendCommand (CMD_ID, 1);

  // wifi connection
  WiFi.begin(ssid, password);

  // waiting for the connectioin
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  // time
  timeClient.begin();
  // UTC +2
  timeClient.setTimeOffset(7200);



  int soilMoistureValue,
      tempCelsius;
  // check WiFi connection
  if (WiFi.status() == WL_CONNECTED && resp) {
    //calling function for request and save date and time
    timeFunction();
    // stack with fixed size
    StaticJsonBuffer<300> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();

    delay(500);
    // ask the value of the moisture sensor that is connected to A0 pin in Arduino
    sendCommand (CMD_READ_A0, 2);
    soilMoistureValue = Wire.read ();
    soilMoistureValue <<= 8;
    soilMoistureValue |= Wire.read ();
    Serial.print ("Mositure value: ");
    soilMoisturePercentage = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
    Serial.println (soilMoisturePercentage, DEC);

    // ask the value of the temperature sensor that is connected to D2 pin in Arduino
    sendCommand (CMD_READ_D2, 1);
    tempCelsius = Wire.read ();
    Serial.print ("Temperature value: ");
    Serial.println (tempCelsius, DEC);

    // we set a fixed time where we are sure that the sun is alredy set
    // you cannot give water to the plants when the sun is not set because otherwise the water will
    if (hour < 5 || hour >= 21) {
      // if the moisture is lower than the optimal value of water we have to give the water
      if (soilMoisturePercentage < optimalWaterValue) {
        // tell to Arduino to turn on the valve
        Wire.beginTransmission(SLAVE_ADDRESS); 
        Wire.write(CMD_ON);        // send value_pot
        Wire.endTransmission();
        delay(14000);
        Wire.beginTransmission(SLAVE_ADDRESS); 
        Wire.write(CMD_OFF);        // send value_pot
        Wire.endTransmission();

      } else {
        Serial.println("P:No more water for me");
      }
    }
    // if the sun is still not set we cannot check the moisture level
    else {
      // if the moisture is lower than the optimal value of water we have to give the water
      if (soilMoisturePercentage < optimalWaterValue)
        Serial.println("It's not time to give water");
    }

    // if moisture error set the value to 120
    if ((soilMoisturePercentage < 0) || (soilMoisturePercentage > 100)) {
      root["moist"] = 120;
    } else {
      root["moist"] = soilMoisturePercentage;
    }
    // if the value of temperature is greater than an error occur
    if (tempCelsius > 55) {
      root["temp"] = 120;
    } else {
      root["temp"] = tempCelsius;
    }
    root["timestamp"] = dateAndTime;

    String databuf;
    root.printTo(databuf);

    //Declare object of class HTTPClient
    HTTPClient http;
    http.begin(serverName);
    //content-type header
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(String(databuf));
    String payload = http.getString();
    Serial.println();
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(payload);    //Print request response payload
    // Disconnect
    http.end();
  }
  else {
    Serial.println("no response");
  }
Serial.println();
  Serial.println("I'm awake, but I'm going into deep sleep mode for 29 minutes");
  ESP.deepSleep(300e6);
  //ESP.deepSleep(30e6); 
}

void loop()
{}

void sendCommand (const byte cmd, const int responseSize)
{
  Wire.beginTransmission (SLAVE_ADDRESS);
  Wire.write (cmd);
  Wire.endTransmission ();

  if (Wire.requestFrom (SLAVE_ADDRESS, responseSize) == 0)
  {
    // no response
    resp = false;
  } else {
    resp = true;
  }
}

void timeFunction() {
  // update the code for not have to many error
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  //set year
  currentYear = ptm->tm_year + 1900;
  //Day in the month
  monthDay = ptm->tm_mday;
  //month
  int currentMonth = ptm->tm_mon + 1;
  currentMonthName = months[currentMonth - 1];
  nameDay = daysOfTheWeek[timeClient.getDay()];
  //time
  hour = timeClient.getHours();
  minute = timeClient.getMinutes();
  seconds = timeClient.getSeconds();

  dateAndTime = String(nameDay + ',' + hour + ':' + minute);
}
