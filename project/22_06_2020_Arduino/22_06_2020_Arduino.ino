#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2 // Data wire is plugged into D2 pin
#define TEMPERATURE_PRECISION 9

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensor(&oneWire);
DeviceAddress one;

const byte MY_ADDRESS = 85;
const int SOLENOID_VALVE_PIN = 8;
//const int waterPumpPin = A3;
int tempC;
// various commands we might get
enum {
  CMD_ID = 1,
  CMD_READ_A0  = 2,
  CMD_READ_D2 = 3,
  CMD_TURN_ON_A2 = 4
};


const int waterDuration = 5000;
char command;

void setup()
{
  command = 0;

  // set the output pin to turn on the transistor
  pinMode(SOLENOID_VALVE_PIN, OUTPUT);
  /*
    pinMode (8, INPUT);
    digitalWrite (8, HIGH);  // enable pull-up
    pinMode (A0, INPUT);
    digitalWrite (A0, LOW);  // disable pull-up
  */
  Wire.begin (MY_ADDRESS);
  Wire.onReceive (receiveEvent);  // interrupt handler for incoming messages
  Wire.onRequest (requestEvent);  // interrupt handler for when data is wanted

  pinMode(8, OUTPUT);
  Serial.begin(9600);
  //temperature
  Serial.println("Dallas Temperature IC Control Library Demo");
  sensor.begin();
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensor.getDeviceCount(), DEC);
  Serial.println(" devices.");
  Serial.print("Parasite power is: ");

  if (sensor.isParasitePowerMode()) {
    Serial.println("ON");
  }
  else {
    Serial.println("OFF");
  }
  if (!sensor.getAddress(one, 0)) {
    Serial.println("Unable to find address for Device 0");
  }


  Serial.print("Device 0 Address: ");
  printAddress(one);
  Serial.println(); Serial.print("Device 1 Address: ");

  sensor.setResolution(one, TEMPERATURE_PRECISION);


  // one tab
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensor.getResolution(one), DEC);
  Serial.println();
}
int temp;
void loop()
{
  Serial.print("Requesting temperatures...");
  sensor.requestTemperatures();
  Serial.println("DONE");
  printData(one);

  delay(1000);
  // all done by interrupts
}  // end of loop

void receiveEvent (int howMany)
{
  Serial.println("receiveEvent");
  // save the command send by NodeMCU
  command = Wire.read ();
  Serial.println("receiveEventEnd");
} // end of receiveEvent


void sendSensor (const byte analogValueRead, const int num)
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
void requestEvent ()
{

  switch (command)
  {
    case CMD_ID:
      Wire.write (0x55);
      break;
    case CMD_READ_A0:
      sendSensor (A0,tempC);
      break;  // send moisture value
    case CMD_READ_D2:
    temptWrite(tempC);
    Serial.println("SII");
      Serial.println(tempC);
      break;
    // send temperature value
    case CMD_TURN_ON_A2:
    Serial.println("TUTTOOOO on");
      solenoidValveOn();
      delay(waterDuration);
      solenoidValveOff();

      // we have to turn on the solenoid valve
      break;
  }  // end of switch

}  // end of requestEvent

void solenoidValveOn() {
  digitalWrite(SOLENOID_VALVE_PIN, HIGH);
}

void solenoidValveOff() {
  digitalWrite(SOLENOID_VALVE_PIN, LOW);
}


void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void printResolution(DeviceAddress deviceAddress) {
  Serial.print("Resolution: ");
  Serial.println(sensor.getResolution(deviceAddress));
}
void printData(DeviceAddress deviceAddress) {
  tempC = sensor.getTempC(deviceAddress);
 
  printAddress(deviceAddress);
}
