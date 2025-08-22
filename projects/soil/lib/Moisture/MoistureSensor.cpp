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

/// @brief begin communication with the moisture sensor
/// @return The wiring or sensor is suspect if the sensor returns 0 for readings.

MoistureSensor::MOISTURE_ERROR MoistureSensor::begin()
{
    MOISTURE_ERROR err = ERR_NO_ERROR;
    pinMode(analogPin, INPUT);

    // TODO: Insert a sanity test for the wiring.
    // Read a value and see if it is reasonable
    return(err);
}

void MoistureSensor::read()
{
    current();
}

int MoistureSensor::current()
{
    lastReading = readAveraged(analogPin);
    return lastReading;
}

int MoistureSensor::readAveraged(int pin)
{
    int readings = 0;
    for (int i = 0; i <= sampleSize; i++)
    {
        readings += analogRead(pin);
        delay(10);
    }
    return (int(readings / sampleSize));
};
