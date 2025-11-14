//
// Read a soil sensor and write the value out to console.
//
#include "Arduino.h"

// #include <MoistureSensor.h>
// #include <MoistureSensorCalibration.h>
// #include <RYLR988.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// This is for debugging
//#include "avr8-stub.h"
//#include "app_api.h" // only needed with flash breakpoints


/// Comment out for debug
//MoistureSensor soilSensor(A0);
// MoistureSensorCalibration calibration;
// RYLR988 radio;

// OLED begin
#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
// This works on Arduino IDE
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



// OLED end

// The message
#define MSG_LED DD3
#define RADIO_LED DD4

#define RADIO_BAUD    115200
#define RADIO_ID      3
#define RADIO_NETWORK 18

// Debug
//RYLR988::SERIAL_ERR radioError;
//MoistureSensor::MOISTURE_ERROR sensorError;



// Output buffer
char buffer[80];

void setup() {

  // The built-in LED will be used to indicate that there is a problem
  //pinnMode(LED_BUILTIN, OUTPUT);

  // The LED attaced to D3 will be illuminated with each message
  //pinMode(DD3, OUTPUT);
    // The LED attaced to D3 will be illuminated with each message
  //pinMode(DD4, OUTPUT);

  delay(1000);
  Serial.begin(115200);
  Serial.println("Start screen");
  // Original
  display.begin(i2c_Address, true);
  Serial.println("Screen started");

  // Ths moisture sensor

  //sensorError = soilSensor.begin();

  // Check to see if things went OK
  // debug
  // if (sensorError != MoistureSensor::ERR_NO_ERROR) {
  //   Serial.begin(115200);
  //   sprintf(buffer, "The sensor or wiring to pin (%d) is suspect", A6);
  //   Serial.println(buffer);
  //   return;
  // }


  // The radio
  // debug
  //radioError = radio.begin(RADIO_BAUD, RADIO_ID, RADIO_NETWORK);
  return;

  // Check to see of things went OK
  // debug
  // if (radioError != RYLR988::ERR_NO_ERROR) {
  //   Serial.begin(115200);
  //   sprintf(buffer, "Radio initialization failed with error: %s", radio.errorString(radioError).c_str());
  //   Serial.println(buffer);
  //   return;
  // }
  // else {
  //   Serial.println("Radio Initialized");
  // }

  //sprintf(output, "Size of EEPROM: %d", calibration.length());
  // calibration.read();
  // sprintf(output, "Wetpoint: %d\nDryPoint: %d\n", calibration.getWetPoint(), calibration.getDryPoint());
  // Serial.println(output);
}

void loop() {
  char readings[80];
  //int currentLevel = soilSensor.current();


  // Check the error state, and flash the LED if needed
  // if (radioError == RYLR988::ERR_TX){
  //   digitalWrite(RADIO_LED, HIGH);
  //   delay(100);
  //   digitalWrite(RADIO_LED, LOW);
  //   delay(100);
  // }
  // If there is an incoming command or request, process that
  // else if (Serial.available() > 0) {
  //   radio.waitMsg(buffer, sizeof(buffer), 0);
  //   radio.processMsg(buffer)
  // }
  // Otherwise, send the current reading and blink the LED


}