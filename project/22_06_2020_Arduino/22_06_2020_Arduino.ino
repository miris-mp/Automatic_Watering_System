#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
// Pin of temperature sensor
#define TEMPERATURE_PIN 2 
// The temperature precision can be between 9 and 12
#define TEMPERATURE_PRECISION 9

OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature temp(&oneWire);
DeviceAddress one;

const byte MY_ADDRESS = 85;
const int SOLENOID_VALVE_PIN = 8;

int tempC;

// various commands we might get
enum {
  CMD_ID = 1,
  CMD_READ_A0  = 2,
  CMD_READ_D2 = 3,
  CMD_TURN_ON_A2 = 4
};

// milliseconds when the solenoid water should provide water to the plant
const int waterDuration = 10000;
char command;

void setup()
{
  command = 0;

  // set the output pin to turn on the transistor
  Wire.begin (MY_ADDRESS);
  Wire.onReceive (receiveEvent);  
  // interrupt handler for incoming messages
  // interrupt handler for when data is wanted
  Wire.onRequest (requestEvent);  

  Serial.begin(9600);
  pinMode(SOLENOID_VALVE_PIN, OUTPUT);
  //temperature
  
  temp.begin();
  Serial.print(temp.getDeviceCount(), DEC);

  if (temp.isParasitePowerMode()) {
    Serial.println("ON");
  }
  else {
    Serial.println("OFF");
  }
  if (!temp.getAddress(one, 0)) {
    Serial.println("Unable to find address for Device 0");
  }

  Serial.print("Device 0 Address: ");
  printAddress(one);
  Serial.println(); Serial.print("Device 1 Address: ");

  temp.setResolution(one, TEMPERATURE_PRECISION);


  // one tab
  Serial.print("Device 0 Resolution: ");
  Serial.print(temp.getResolution(one), DEC);
  Serial.println();
}
int temp;

void loop()
{
  temp.requestTemperatures();
    printData(one);
} 

void receiveEvent (int howMany)
{
  // save the command send by NodeMCU
  command = Wire.read ();
  // check if the request is the one for the solenoid valve and in that case handle here
} 

void requestEvent ()
{
  switch (command)
  {
    case CMD_ID:
      Wire.write (0x55);
      break;
    case CMD_READ_A0:
      sendSensor (A0);
      break;  // send moisture value
    case CMD_READ_D2:
    temptWrite(tempC);
    Serial.println("SII");
      Serial.println(tempC);
      break;
    // send temperature value
    case CMD_TURN_ON_A2:
    digitalWrite(SOLENOID_VALVE_PIN, HIGH);      //Switch Solenoid ON
  delay(5000);                          //Wait 1 Second
  digitalWrite(SOLENOID_VALVE_PIN, LOW);  

      // we have to turn on the solenoid valve
      break;
  }  // end of switch

} 

void sendSensor (const byte analogValueRead)
{
  int value = analogRead(analogValueRead);
  byte buf [2];
  buf [0] = value >> 8;
  buf [1] = value & 0xFF;
  Wire.write(buf,sizeof(buf));
}  // end of sendSensor

void temptWrite (const int num) {
    byte buf [2];
  buf [0] = num ;

  Wire.write (buf, 1);
}

void solenoidValveOn() {
  digitalWrite(SOLENOID_VALVE_PIN, HIGH);
  delay(3000);
}

void solenoidValveOff() {
  digitalWrite(SOLENOID_VALVE_PIN, LOW);
  delay(5000);
}


void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void printResolution(DeviceAddress deviceAddress) {
  Serial.print("Resolution: ");
  Serial.println(temp.getResolution(deviceAddress));
}
void printData(DeviceAddress deviceAddress) {
  tempC = temp.getTempC(deviceAddress);
 
  printAddress(deviceAddress);
}
