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
char* ssid = "Redmi";
char* password = "p4ssw0rd";

char* serverName = "http://salahezz.pythonanywhere.com/postjson";


//The udp library class
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
String dateAndTime = "";
int months[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
String day = "";
char daysOfTheWeek[7][12]= {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
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
const int waterLowerBound = 20;
const int waterUpperBound = 25;
const int dry = 520;
const int wet = 270;

int soilMoistureValue = 0;
int soilMoisturePercentage = 0;
int soilTemperatureValue = 0;
int soilTemperaturePercentage = 0;

const int SLAVE_ADDRESS = 85;

// various commands we might send
enum {
  CMD_ID = 1,
  CMD_READ_A0  = 2,
  CMD_READ_D2 = 3,
  CMD_TURN_ON_A2 = 4
};

//Telegram connection
/*
  CTBot myBot;
  String token = "1176180431:AAEQfRsYrJMLfx3QDsC3h--WqL3xnUQpdBc";   // REPLACE myToken WITH YOUR TELEGRAM BOT TOKEN
  uint8_t led = 2;            // the onboard ESP8266 LED.
*/


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
  timeClient.setTimeOffset(7200);

  /*
    // connect the ESP8266 to the desired access point
    myBot.wifiConnect(ssid, password);

    // set the telegram bot token
    myBot.setTelegramToken(token);

    // check if all things are ok
    if (myBot.testConnection())
      Serial.println("\ntestConnection OK");
    else
      Serial.println("\ntestConnection NOK");

    // set the pin connected to the LED to act as output pin
    pinMode(led, OUTPUT);
    digitalWrite(led, HIGH); // turn off the led (inverted logic!)
  */
}  // end of setup

void loop()
{
  int val,
  tempCelsius;

  // stack with fixed size
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  delay(500);
 sendCommand (CMD_READ_A0, 2);
  val = Wire.read ();
  val <<= 8;
  val |= Wire.read ();
  Serial.print ("Mositure value: ");
  soilMoistureValue = val;
  soilMoisturePercentage = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  Serial.println (soilMoisturePercentage, DEC);

  sendCommand (CMD_READ_D2, 1);
  tempCelsius = Wire.read ();
  Serial.print ("Temperature value: ");
  Serial.println (tempCelsius, DEC);
  
  Serial.println("acqus ");
  Serial.println(hour);
  if (hour > 21) {
    Serial.println("Rancore ");
    // if the moisture is lower than the optimal value of water we have to give the water
    if (soilMoisturePercentage < waterLowerBound) {
      Serial.println("Canta ");
      // tell to Arduino to turn on the valve
      sendCommand(CMD_TURN_ON_A2, 3);
    } else {
      Serial.println("The water level are good");
    }
  }
  // if the moisture is not lower than the lower bound tell the water value is okay
   else {
      Serial.println("It's not time to give water");
    }
  
  delay(500);
 
  // check WiFi connection
  if (WiFi.status() == WL_CONNECTED) {
    //calling function for request and save date and time
    timeFunction();
    root["timestamp"] = dateAndTime;
    root["moist"] = soilMoisturePercentage;
    root["temp"] = tempCelsius;
    String databuf;
    root.printTo(databuf);

    //Declare object of class HTTPClient
    HTTPClient http;
    http.begin(serverName);
//content-type header
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(String(databuf));
    String payload = http.getString();

    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(payload);    //Print request response payload
    Serial.println(dateAndTime);
    // Disconnect
    http.end();
  } 
}

void sendCommand (const byte cmd, const int responseSize)
{
  Wire.beginTransmission (SLAVE_ADDRESS);
  Wire.write (cmd);
  Wire.endTransmission ();

  if (Wire.requestFrom (SLAVE_ADDRESS, responseSize) == 0)
  {
    // handle error - no response
    Serial.print("no response");
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

  dateAndTime = String(nameDay+',' + hour + ':' + minute);
}
