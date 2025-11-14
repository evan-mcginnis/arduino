//
// G Y 6 1   A C C E L E R O M E T E R
//

#include "Arduino.h"

class GY61 {
  public:
    GY61(int xPin, int yPin, int zPin);

    bool read();

    bool begin();

    // Get the sample size 
    int getSampleSize();

    // Set the sample size
    int setSampleSize(int samples);


    float x();

    float y();

    float z();


  private:
    bool begun;

    int pinX;
    int pinY;
    int pinZ;

    int rawX;
    int rawY;
    int rawZ;

    float xAccel;
    float yAccel;
    float zAccel;

    // The previous readings
    float prior_x = 0.0;
    float prior_y = 0.0;
    float prior_z = 0.0;


    int minX = 0;
    int maxX = 0;
    int minY = 0;
    int maxY = 0;
    int minZ = 0;
    int maxZ = 0;

    int rawMax = 0;

    int sampleSize = 10;

    float readAveraged(int pin);

};
