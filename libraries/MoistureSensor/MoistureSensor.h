//
// Moisture sensor
// Evan McGinnis
// 8 Aug 2025
//
#ifndef SOIL_h
#define SOIL_h

#include "Arduino.h"

class MoistureSensor {
  public:
    MoistureSensor(int pin);

    void begin();
    void read();
    float current();
  
  private:
    int analogPin;
    int sampleSize = 10;
    float lastReading;

    float readAveraged(int pin);
};

#endif
