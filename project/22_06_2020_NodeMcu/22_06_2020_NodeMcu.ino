#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
#include <ArduinoJson.h>

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

void setup() {
  // wifi connection
  WiFi.begin(ssid, password);
  Serial.begin(9600);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("No no no");
  // time
  timeClient.begin();
  timeClient.setTimeOffset(7200);
}

void loop() {
  // check WiFi connection
  if (WiFi.status() == WL_CONNECTED && Serial.available()) {
    //calling function for request and save date and time
    timeFunction();

    StaticJsonBuffer<300> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(Serial);
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

    Serial.println(payload);
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(payload);    //Print request response payload
    // Disconnect
    http.end();
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
  //time
  hour = timeClient.getHours();
  minute = timeClient.getMinutes();
  seconds = timeClient.getSeconds();

  dateAndTime = String(currentYear) + '-' + currentMonthName + '-' + monthDay + ' ' + hour + ':' + minute + ':' + seconds;
}
