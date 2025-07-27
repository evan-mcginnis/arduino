/*
Reads the x,y,z pins of a Gy-61 device, using a threshold to detect change.


*/

// The previous readings
float prior_x = 0.0;
float prior_y = 0.0;
float prior_z = 0.0;



class Accelerometer{
  public:
    Accelerometer(int xPin, int yPin, int zPin)
    {
      pinX = xPin;
      pinY = yPin;
      pinZ = zPin;
    };

    int read()
    {
      int rawX = readAveraged(pinX);
      int rawY = readAveraged(pinY);
      int rawZ = readAveraged(pinZ);

      long xScaled = map(rawX, minX, maxX, -3000, 3000);
      long yScaled = map(rawY, minY, maxY, -3000, 3000);
      long zScaled = map(rawZ, minZ, maxZ, -3000, 3000);

      xAccel = xScaled/1000.0;
      yAccel = yScaled/1000.0;
      zAccel = zScaled/1000.0;
      // xAccel = xScaled;
      // yAccel = yScaled;
      // zAccel = zScaled;
    };

    // Get the sample size 
    int getSampleSize()
    {
      return sampleSize;
    };

    // Set the sample size
    int setSampleSize(int samples)
    {
      sampleSize = samples;
    };

    void setCalibratedX(int min, int max)
    {
      minX = min;
      maxX = max;
    }

    void setCalibratedY(int min, int max)
    {
      minY = min;
      maxY = max;
    }

    void setCalibratedZ(int min, int max)
    {
      minZ = min;
      maxZ = max;
    }

    float x()
    {
      return xAccel;
    }

    float y()
    {
      return yAccel;
    }

    float z()
    {
      return zAccel;
    }


  private:
    int pinX;
    int pinY;
    int pinZ;

    int rawX;
    int rawY;
    int rawZ;

    float xAccel;
    float yAccel;
    float zAccel;

    int minX = 0;
    int maxX = 0;
    int minY = 0;
    int maxY = 0;
    int minZ = 0;
    int maxZ = 0;

    int sampleSize = 10;

    //
    // Read the pin over the specified number of samples
    // Subsequent readings may differ slightly, so average them
    //

    float readAveraged(int pin)
    {
      int readings = 0;
      for(int i = 0; i <= sampleSize; i++)
      {
        readings += analogRead(pin);
        delay(10);
      }
      return (readings/sampleSize);
    };

};

Accelerometer* accel = new Accelerometer(A2, A0, A1);

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  


  // Set the calibration
  accel->setCalibratedX(200, 500);
  accel->setCalibratedY(200, 500);
  accel->setCalibratedZ(200, 500);
}

//
// Check to see if the values differ by the tolerance allowed
//

int differs(float value, float standard, float tolerance)
{
  int different = 0;
  different = abs(value - standard) > tolerance;
  return different;
}



void loop() {
  char readings[80];
  // The string versions of the readings
  char x_f[10];
  char y_f[10];
  char z_f[10];
  // The threshold to define movement in an axis
  float threshold = 2.0;



  // Read the pins
  accel->read();


  // Check to see if any of them differ greater than the threshold
  if (differs(accel->x(), prior_x, threshold) || differs(accel->y(), prior_y, threshold) || differs(accel->z(), prior_z, threshold)) {
      dtostrf(accel->x(), 6, 2, x_f);
      dtostrf(accel->y(), 6, 2, y_f);
      dtostrf(accel->z(), 6, 2, z_f);

      // Indicate that there was movement my flashing the LED
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100); 
      digitalWrite(LED_BUILTIN, LOW);
  }
  // sprintf(readings, "(%s,%s,%s)", x_f, y_f, z_f);
  // Serial.println(readings);
  delay(1000);
  //sprintf(readings, "x:%s", x_f);
  //Serial.println(readings);
  sprintf(readings, "y:%s", y_f);
  Serial.println(readings);
  // sprintf(readings, "z:%s", z_f);
  // Serial.println(readings);
  
  // Store the current readings as the prior readings
  prior_x = accel->x();
  prior_y = accel->y();
  prior_z = accel->z();

  //   // turn the LED on (HIGH is the voltage level)
  //  
  // Serial.println("off");                    // wait for a second
  // digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  // delay(1000);                      // wait for a second
}
