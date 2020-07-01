#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
//#include "CTBot.h"

// wifi access
char* ssid = "Redmi";
char* password = "p4ssw0rd";
// Domain Name with full URL Path for HTTP POST Request
const char* serverName = "http://api.thingspeak.com/update";
// Service API Key
String apiKey = "UXYEVVEWKC4DAGIP";  
//char* serverName = "http://miris.pythonanywhere.com/postjson";


//The udp library class
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
String dateAndTime = "";
int months[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
String day = "";
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

int soilMoistureValue = 0;
int soilMoisturePercentage = 0;
int soilTemperatureValue = 0;
int soilTemperaturePercentage = 0;

const int SLAVE_ADDRESS = 42;

// various commands we might send
enum {
  CMD_ID = 1,
  CMD_READ_A0  = 2
  //CMD_READ_D8 = 3
};

//Telegram connection
/*
CTBot myBot;
String token = "1176180431:AAEQfRsYrJMLfx3QDsC3h--WqL3xnUQpdBc";   // REPLACE myToken WITH YOUR TELEGRAM BOT TOKEN
uint8_t led = 2;            // the onboard ESP8266 LED.
*/
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
  else
  {
    // data received, now use Wire.read to obtain it
  }
}  // end of sendCommand

void setup ()
{
  Serial.begin(9600);
  Wire.begin(D1, D2);

  sendCommand (CMD_ID, 1);

  // wifi connection
  WiFi.begin(ssid, password);

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
  StaticJsonBuffer<300> jsonBuffer;
  //JsonObject& root = jsonBuffer.parseObject(Serial);
  JsonObject& root = jsonBuffer.createObject();

  sendCommand (CMD_READ_A0, 2);
  soilMoistureValue = Wire.read ();
  soilMoistureValue <<= 8;
  soilMoistureValue |= Wire.read ();
  Serial.print ("Value of A0: ");
  Serial.println (soilMoistureValue, DEC);
  
  soilMoisturePercentage = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  

  //sendCommand (CMD_READ_D8, 3);
  /*
  val = Wire.read ();
  Serial.print ("Value of D8: ");
  Serial.println (val, DEC);
  */

  delay (500);
  // check WiFi connection
  if (WiFi.status() == WL_CONNECTED) {
    //calling function for request and save date and time
    timeFunction();

    root["timestamp"] = dateAndTime;
    root["sensorType"] = "moisture";
    root["moistureValue"] = soilMoistureValue;
    root["moisturePercentage"] = soilMoisturePercentage;
    String databuf;
    root.printTo(databuf);

    //Declare object of class HTTPClient
    HTTPClient http;
    http.begin(serverName);
/*
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(String(databuf));
    String payload = http.getString();
    */
    //content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
     // Data to send with HTTP POST
      String httpRequestData = "api_key=" + apiKey + "&field1=" + String(soilMoistureValue) + "&field2=" + String(soilMoisturePercentage);       
      
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);
      
    int httpCode = http.POST(String(databuf));
    String payload = http.getString();

    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(payload);    //Print request response payload
    // Disconnect
    http.end();
  }

  // a variable to store telegram message data
 /* TBMessage msg;

  // if there is an incoming message...
  if (myBot.getNewMessage(msg)) {

    if (msg.text.equalsIgnoreCase("LIGHT ON")) {              // if the received message is "LIGHT ON"...
      digitalWrite(led, LOW);                               // turn on the LED (inverted logic!)
      myBot.sendMessage(msg.sender.id, "Light is now ON");  // notify the sender
    }
    else if (msg.text.equalsIgnoreCase("LIGHT OFF")) {        // if the received message is "LIGHT OFF"...
      digitalWrite(led, HIGH);                              // turn off the led (inverted logic!)
      myBot.sendMessage(msg.sender.id, "Light is now OFF"); // notify the sender
    }
    else {                                                    // otherwise...
      // generate the message for the sender
      String reply;
      reply = (String)"Welcome " + msg.sender.username + (String)". Try LIGHT ON or LIGHT OFF.";
      myBot.sendMessage(msg.sender.id, reply);             // and send it
    }
  }
  */
  // wait 500 milliseconds
  delay(500);
}

/********************************************/
/* old Arduino part
  // Memory pool for JSON object tree. Size in bytes
  StaticJsonBuffer<300> jsonBuffer;   //Memory pool
  JsonObject&  root = jsonBuffer.createObject();
  root["sensorType"] = "moisture";
  root["moistureValue"] = soilMoistureValue;
  root["moisturePercentage"] = soilMoisturePercentage;
*/
/********************************************/


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
  //time
  hour = timeClient.getHours();
  minute = timeClient.getMinutes();
  seconds = timeClient.getSeconds();

  dateAndTime = String(currentYear) + '-' + currentMonthName + '-' + monthDay + ' ' + hour + ':' + minute + ':' + seconds;
}
