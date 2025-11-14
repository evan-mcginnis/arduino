//
// G Y 6 1   A C C E L E R O M E T E R
//

#include "GY61.h"


/// @brief GY61 Accelerometer (requires begin() to be called before reading)
/// @param xPin Pin for X
/// @param yPin Pin for Y
/// @param zPin Pin for Z
GY61::GY61(int xPin, int yPin, int zPin)
{
  pinX = xPin;
  pinY = yPin;
  pinZ = zPin;

  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  pinMode(zPin, INPUT);
  
  this->sampleSize = 10;

  this->begun = false;

// The ESP32 has 12 bit resolution, and Arduino Nano 10
#ifdef ESP32
  #define ANALOG_MAX 4095
#else 
  #define ANALOG_MAX 1023
#endif

  this->minX = 0;
  this->maxX = ANALOG_MAX;
  this->minY = 0;
  this->maxY = ANALOG_MAX;
  this->minZ = 0;
  this->maxZ = ANALOG_MAX;

};

/// @brief Begin the GY61
/// @return 
bool GY61::begin()
{
  this->begun = true;
  return this->begun;
}

/// @brief Read the GY61, averaging the readings
/// @return 
bool GY61::read()
{
  bool rc = false;

  if (begun){
    // int rawX = readAveraged(pinX);
    // int rawY = readAveraged(pinY);
    // int rawZ = readAveraged(pinZ);

    int rawX = analogRead(pinX);
    int rawY = analogRead(pinY);
    int rawZ = analogRead(pinZ);

    //Serial.printf("Raw: (%d, %d, %d)\n", rawX, rawY, rawZ);

    long xScaled = map(rawX, minX, maxX, -3000, 3000);
    long yScaled = map(rawY, minY, maxY, -3000, 3000);
    long zScaled = map(rawZ, minZ, maxZ, -3000, 3000);

    xAccel = xScaled/1000.0;
    yAccel = yScaled/1000.0;
    zAccel = zScaled/1000.0;

    rc = true;
  }

  return rc;
};


/// @brief Get the sample size used for the averaged reads
/// @return sample size
int GY61::getSampleSize()
{
  return sampleSize;
};


/// @brief Set the sample size
/// @param number of samples to use
/// @return number of samples to use
int GY61::setSampleSize(int samples)
{
  sampleSize = samples;
  return samples;
};



float GY61::x()
{
  return xAccel;
}

float GY61::y()
{
  return yAccel;
}

float GY61::z()
{
  return zAccel;
}



//
// Read the pin over the specified number of samples
// Subsequent readings may differ slightly, so average them
//

float GY61::readAveraged(int pin)
{
  int readings = 0;
  for(int i = 0; i <= sampleSize; i++)
  {
    readings += analogRead(pin);
    delay(10);
  }
  return (readings/sampleSize);
};


