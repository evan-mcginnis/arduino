//
// A demo for how to use the Moisture Sensor 
//

#include <MoistureSensor.h>

MoistureSensor soilSensor(A6);

void setup() {
  soilSensor.begin();
  Serial.begin(115200);
}

void loop() {
  char readings[80];
  int currentLevel = soilSensor.current();

  sprintf(readings, "moisture:%d", currentLevel);
  Serial.println(readings);
}
