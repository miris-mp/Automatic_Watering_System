
// Written by Nick Gammon
// Date: 18th February 2011

#include <Wire.h>

const byte MY_ADDRESS = 42;

// various commands we might get
enum {
  CMD_ID = 1,
  CMD_READ_A0  = 2,
  CMD_READ_D8 = 3
};


// we have to take the values of the sensor for the calculation it's needed the airValue and the WaterValue as fix parameter
const int AirValue = 620;   //you need to replace this value with Value_1
const int WaterValue = 270;

// generic value for the water to give
int optimalMoistureValue = 600;

// set the pin for the transistor
int motorPin = 13;

char command;

void setup()
{
  command = 0;

  // set the output pin to turn on the transistor
  pinMode(motorPin, OUTPUT);
  /*
    pinMode (8, INPUT);
    digitalWrite (8, HIGH);  // enable pull-up
    pinMode (A0, INPUT);
    digitalWrite (A0, LOW);  // disable pull-up
  */
  Wire.begin (MY_ADDRESS);
  Wire.onReceive (receiveEvent);  // interrupt handler for incoming messages
  Wire.onRequest (requestEvent);  // interrupt handler for when data is wanted

  Serial.begin(9600);
}  // end of setup


void loop()
{
  // all done by interrupts
}  // end of loop

void receiveEvent (int howMany)
{
  command = Wire.read ();  // remember command for when we get request
} // end of receiveEvent


void sendSensor (const byte analogValueRead)
{
  int value = analogRead(analogValueRead);
  byte buf [2];

  buf [0] = value >> 8;
  buf [1] = value & 0xFF;
  Wire.write (buf, 2);
}  // end of sendSensor

void requestEvent ()
{
  switch (command)
  {
    case CMD_ID:      Wire.write (0x55); break;   // send our ID
    case CMD_READ_A0: sendSensor (A0); break;  // send A0 value
    case CMD_READ_D8: sendSensor (A0);  // send A1 value
  }  // end of switch

}  // end of requestEvent
