#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>

// wifi access
char* ssid = "Redmi";
char* password = "p4ssw0rd";
char* serverName = "http://miris.pythonanywhere.com/postjson";

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
  CMD_READ_A0  = 2,
  CMD_READ_D8 = 3
};

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

}  // end of setup

void loop()
{
  int val;
  StaticJsonBuffer<300> jsonBuffer;
  //JsonObject& root = jsonBuffer.parseObject(Serial);
  JsonObject& root = jsonBuffer.createObject();
  
  sendCommand (CMD_READ_A0, 2);
  val = Wire.read ();
  val <<= 8;
  val |= Wire.read ();
  Serial.print ("Value of A0: ");
  soilMoistureValue = val;
   soilMoisturePercentage = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  Serial.println (val, DEC);

  sendCommand (CMD_READ_D8, 3);
  val = Wire.read ();
  Serial.print ("Value of D8: ");
  Serial.println (val, DEC);

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
    //content-type header
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(String(databuf));
    String payload = http.getString();

    Serial.println(payload);
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(payload);    //Print request response payload
    // Disconnect
    http.end();
  }
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
