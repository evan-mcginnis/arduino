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
    // This is a dumb sensor, so if we get a 0 or 1 for a read, it's suspect, or the wiring is wrong
    enum MOISTURE_ERROR {
        ERR_NO_ERROR,
        ERR_SUSPECT
    };
    MoistureSensor(int pin);

    MOISTURE_ERROR begin();
    void read();
    int current();

  private:
    int analogPin;
    int sampleSize = 10;
    int lastReading;

    int readAveraged(int pin);
};

#endif
