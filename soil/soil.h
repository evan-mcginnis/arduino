#ifndef SOIL_h
#define SOIL_h

class MoistureSensor {
  public:
    MoistureSensor(int pin);

    void read();
    float current();
  
  private:
    int analogPin;
    int sampleSize = 10;
    float lastReading;

    float readAveraged(int pin);
}

#endif
