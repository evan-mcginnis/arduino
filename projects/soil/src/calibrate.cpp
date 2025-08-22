// #include <MoistureSensor.h>
// #include <MoistureSensorCalibration.h>

// MoistureSensor soilSensor(A6);
// MoistureSensorCalibration calibration;

// void setup() {
//   char output[80];

//   soilSensor.begin();
//   Serial.begin(115200);

// }

// void loop() {
//   char readings[80];
//   int currentLevel = soilSensor.current();

//   Serial.println("Insert the sensor into dry sample. Press any key to accept");
//   while (!Serial.available()){
//     currentLevel = soilSensor.current();
//     sprintf(readings, "moisture:%d", currentLevel);
//     Serial.println(readings);
//   }


