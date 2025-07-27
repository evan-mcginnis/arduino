//
// A soil moisture sensor
//

class MoistureSensor {
  public:
    MoistureSensor(int pin)
    {
      analogPin = pin;
      pinMode(analogPin, INPUT);
    }

    void read()
    {
      current();
    }

    float current()
    {
      lastReading = readAveraged(analogPin);
      return lastReading;
    }

  private:
    int analogPin;
    int sampleSize = 10;
    float lastReading;

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

}

//
// Main Program
//

// Create a new sensor read with analog port A1
soilSensor = MoistureSensor(A6);

void setup() {
  // Initiailize the console
  Serial.begin(115200);

  // Initialize the LED so we can indicate wetness
  pinMode(LED_BUILTIN, OUTPUT);
}

// This is the main loop that gets excuted repeatedly
void loop() {
  char readings[80];

  // Read the current sensor value
  int currentMoistureLevel = soilSensor.current();
  // Delay 1 second
  delay(1000);

  // Flash the LED to indicate wetness
  if (currentMoistureLevel < 400)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
  }

  // Show this on the console. Don't put a space after the colon, or the plotter won't work correctly
  // sprintf(readings, "moisture:%d", currentMoistureLevel);
  // Serial.println(readings);
  Serial.write(currentMoistureLevel);
}
