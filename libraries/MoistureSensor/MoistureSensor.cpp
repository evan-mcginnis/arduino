//
// A soil moisture sensor
//

#include "Arduino.h"
#include "MoistureSensor.h"


MoistureSensor::MoistureSensor(int pin)
{
  analogPin = pin;
  pinMode(analogPin, INPUT);
}

void MoistureSensor::begin()
{
  pinMode(analogPin, INPUT);
}

void MoistureSensor::read()
{
  current();
}

float MoistureSensor::current()
{
  lastReading = readAveraged(analogPin);
  return lastReading;
}

float MoistureSensor::readAveraged(int pin)
{
  int readings = 0;
  for(int i = 0; i <= sampleSize; i++)
    {
      readings += analogRead(pin);
      delay(10);
    }
  return (readings/sampleSize);
};






