// 
// D R O N E
//
// This will report position every meter (x. y, or z)
//

#include <Arduino.h>
#include <TimerOne.h>

// OLED 
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "OLED.h"

// Radio
#include <RYLR998.h>

// Temp Sensor
#include <Adafruit_AHTX0.h>
Adafruit_AHTX0 aht;

// GNSS
#include <TinyGPS++.h>
TinyGPSPlus gnss;

// OLED 
OLED display;

// Radio
#define RADIO_BAUD    115200
#define RADIO_ID      3       // 0 indicates the library will choose.  Set this to a specific value if needed
#define RADIO_NETWORK 18

// Accelerometer
// #include "MPU6050.h"
// #define MPU6050_ADDR 0x68

// MPU6050 accel(MPU6050_ADDR);

#include <GY61.h>
#define X_PIN D0
#define Y_PIN D1
#define Z_PIN D2
GY61 accel(X_PIN, Y_PIN, Z_PIN);


// See this for using multiple UARTS
// https://forum.seeedstudio.com/t/how-to-use-serial1-with-xiao-esp32c3/266306/4

//Define two Serial devices mapped to the two internal UARTs
HardwareSerial radioSerial(0);
HardwareSerial gnssSerial(1);

// The radio will use TX/RX ports on the XIAO -- D6/D7
// Indicate this with -1 and -1
RYLR988::SERIAL_ERR radioError;
RYLR988 radio(& radioSerial, -1, -1);

// The GNSS will use TX/RX on XIAO D1/D2
#define PIN_GNSS_RX 9
#define PIN_GNSS_TX 10
#define BAUD_GNSS 9600

// Message to send through radio or on OLED
char buffer[128];

// Formatting options for float to string conversion
#define WIDTH_LAT           7
#define WIDTH_LONG          9
#define WIDTH_ALTITUDE      5
#define PRECISION_LAT_LONG  5
#define PRECISION_ALTITUDE  2
#define WIDTH_TEMP          5
#define WIDTH_HUMIDITY      5
#define PRECISION_TEMP_HUMIDITY 3

// THRESHOLDS for travel distance
#ifdef IN_FIELD
#define THRESHOLD_XY  1
#define THRESHOLD_Z   1
#else
#define THRESHOLD_XY 0
#define THRESHOLD_Z 0
#endif

// What we are doing 
enum class STATE {
  waiting = 0,    // Waiting for location data to settle down
  ready = 1,      // Ready for flight
  acquiring = 2   // Acquring readings and sending the back
};

// Sprintf causes a crash, so convert float to strings first in these buffers
char bufferLat[32];
char bufferLong[32];
char bufferAltitude[32];
char bufferAGL[32];
char bufferTemp[32];
char bufferHumidity[32];

volatile unsigned long lastMillis = 0L;
volatile unsigned long debounceTime = 5L; // milliseconds
volatile unsigned long doubleClickTime = 750000L; // microseconds
volatile bool waitingForDoubleClick = false;

// Control what information is displayed
SELECTION currentDisplay;

volatile int clickCount = 0;

volatile bool debugMode = true;


/// @brief Process the button press or double-press
/// @brief This is called by the timer expiration for the double=-click timer
void processButton() {
  Serial.println("Process button");

  // Single or double click will happen within the timeframe for a double-click
  switch (clickCount) {
    case 1:
      Serial.println("-- Single click --");
      // The button cycles through the display
      if (currentDisplay == SELECTION::gnss) {
        currentDisplay = SELECTION::armed;
      }
      else {
        currentDisplay = SELECTION::gnss;
        display.forceDisplay(currentDisplay);
      }
      display.screen()->clearDisplay();
      break;
    case 2:
      // A double click arms the device
      Serial.println("-- Double click --");
      display.swapArmedMode();
      break;
    case 3:
      // A triple click swaps debug mode
      Serial.println("-- Triple click --");
      display.swapDebugMode();
      break;
  }

  // Reset everything for the next press
  clickCount = 0;
  waitingForDoubleClick = false;
  Timer1.stop();
}

/// @brief Interrupt handler for push button. Cycles through displays
void pushButton() {
  
  // Debounce the signal and start the timer to detect a double-click
  if ((millis() - lastMillis) >= debounceTime) {
    clickCount += 1;
    Serial.println("-- Button press --");
    // If we aren't waiting for a double-click, start the timer
    if (!waitingForDoubleClick) {
      Timer1.start();
    }
    waitingForDoubleClick = true;
  }
  // Record the time of the button press
  lastMillis = millis();
}

// Custom NMEA fields for some items
TinyGPSCustom pdop(gnss, "GNGSA", 15);
TinyGPSCustom hdop(gnss, "GNGSA", 16);
TinyGPSCustom vdop(gnss, "GNGSA", 17);
TinyGPSCustom GNSSID(gnss, "GNGSA", 18);

void setup() {
  // C O N S O L E
  Serial.begin(115200);

  // Button
  //pinMode(D8, INPUT_PULLUP);
  pinMode(D8, INPUT);
  attachInterrupt(digitalPinToInterrupt(D8), pushButton, FALLING);
  // The button ISR will start this timer to detect a double-click
  Timer1.initialize(doubleClickTime);
  Timer1.attachInterrupt(processButton);
  Timer1.stop();

  // O L E D
  delay(250);           // Give the OLED time to boot
  display.initialize();
  display.splash();
  // Display the position data by default
  currentDisplay = SELECTION::gnss;

  // Sensor
  if (! aht.begin()) {
    display.debug("Could not find AHT10");
    while (1) delay(10);
  }
  display.screen()->setCursor(0,0);
  display.debug("AHT10 found");
  delay(100);
  display.screen()->clearDisplay();

  // R A D I O
  radioError = radio.begin(RADIO_BAUD, RADIO_ID, RADIO_NETWORK);

  display.screen()->setCursor(0,0);
  if (radioError == RYLR988::ERR_NO_ERROR) {
    sprintf(buffer, "Radio ID: %d", radio.getAddress());
    display.debug(buffer);
    delay(100);
  } else {
    display.debug("Radio Init Error");
  }

  // GNSS
  gnssSerial.begin(9600, SERIAL_8N1, PIN_GNSS_RX, PIN_GNSS_TX);

  display.screen()->setCursor(0, 0);
  display.screen()->clearDisplay();
  display.debug("GPS setup");
  delay(100);

  display.screen()->setCursor(0, 0);
  display.screen()->clearDisplay();
  display.screen()->display();

  // GY61

  accel.begin();

}




void loop() {
  sensors_event_t humidity, temp;
  char debugBuffer[80];
  double xyTravelled = 0.0;
  double zTravelled = 0.0;

  // GNSS
#ifdef DEBUG
  if (gnssSerial.available() == 0) {
    Serial.println("No Location Data");
  }
  else{
    Serial.println("Location updated");
  }
#endif

  while (gnssSerial.available()) {
    char token = gnssSerial.read();
    gnss.encode(token);
    // This prints the NMEA statements the GP-02 generates
    //Serial.printf("%c", token);
  }

  if (gnss.location.isUpdated()) {
#ifdef DEBUG
    Serial.print("Latitude: ");
    Serial.println(gnss.location.lat());
    Serial.print("Longitude: ");
    Serial.println(gnss.location.lng());
    if (gnss.satellites.isValid()) {
      Serial.print("Satellites: ");
      Serial.println(gnss.satellites.value());
    }
    if (gnss.altitude.isValid()) {
      Serial.print("Altitude: ");
      Serial.println(gnss.altitude.meters());
    }
#endif
  }

  if (gnss.location.isValid()) {
    //Serial.printf("PDOP: %s HDOP: %s VDOP: %s GNSS ID: %s\n", pdop.value(), hdop.value(), vdop.value(), GNSSID.value());
    sprintf(buffer, "GNSS Location OK");
    display.updateGNSSStatus(true, buffer);
    //Serial.printf("%d/%02d/%02d %d:%0d:%0d\n", gnss.date.year(), gnss.date.month(), gnss.date.day(), gnss.time.hour(), gnss.time.minute(), gnss.time.second());
    display.updateDate(gnss.date.year(), gnss.date.month(), gnss.date.day());
    display.updateTime(gnss.time.hour(), gnss.time.minute(), gnss.time.second());
    
    // Calculate how stable the readings are
    display.calculateDelta(POSITION::latitude, gnss.location.lat());
    display.calculateDelta(POSITION::longitude, gnss.location.lng());
    display.calculateDelta(POSITION::altitude, gnss.altitude.meters());

#ifdef DEBUG
    Serial.printf("Std Deviations: latitude (%lf) longitude (%lf) altitude (%lf)\n", display.stdDeviationLatitude, display.stdDeviationLongitude, display.stdDeviationAltitude);
#endif
 
    display.updateAccuracy(gnss.hdop.hdop(), atof(vdop.value()));
    display.updateLocation(gnss.location.lat(), gnss.location.lng());
    display.updateAltitude(gnss.altitude.meters());
    display.updateSatellites(gnss.satellites.value());
  
    display.displaySelected(currentDisplay);
  }
  else{
    TinyGPSLocation::Mode fix;
    fix = gnss.location.FixMode();
    sprintf(buffer, "Satellites: %d", gnss.satellites.value());
    display.updateGNSSStatus(false, buffer);
    display.displaySelected(currentDisplay);
  }

  location *lastMarkedLocation = display.getMarkedLocation();

  // Determine how far we have travelled from the last reading location without considering altitude (This should be a haversine calculation)
  xyTravelled = gnss.distanceBetween(gnss.location.lat(), gnss.location.lng(), lastMarkedLocation->latitude, lastMarkedLocation->longitude);

  // Determine how far we have travelled in altitude
  zTravelled = std::abs(gnss.altitude.meters() - lastMarkedLocation->altitude);

  accel.read();
  //Serial.printf("Acceleration: (%d,%d,%d) Gryo: (%d, %d, %d) Temp: %d\n", accel.accelX, accel.accelY, accel.accelZ, accel.gyroX, accel.gyroY, accel.gyroZ, accel.temperature);
  //Serial.printf("Acceleration: (%f,%f,%f)\n", accel.x(), accel.y(), accel.z());
  // 
  // A C T I O N
  //
  // If the travel is above the threshold, take a reading and send it out
  if (xyTravelled >= THRESHOLD_XY ||
      zTravelled >= THRESHOLD_Z) {

      // Set the current location as where the reading was taken
      display.markLocation();

        
      // Get the humidity and temperature
      aht.getEvent(&humidity, &temp);

      // sprintf of floating point numbers causes a crash, so convert values to string
      dtostrf(gnss.location.lat(), WIDTH_LAT, PRECISION_LAT_LONG, bufferLat);  
      dtostrf(gnss.location.lng(), WIDTH_LONG, PRECISION_LAT_LONG, bufferLong);



    #ifdef DEBUG
      Serial.printf("Distance travelled: XY %lf Z %lf\n", xyTravelled, zTravelled);
    #endif

      if (gnss.altitude.isValid()) {
        dtostrf(gnss.altitude.meters(), WIDTH_ALTITUDE, PRECISION_ALTITUDE, bufferAltitude);
      }
      else {
        //dtostrf(display.lastAltitude, 4, 2, bufferAltitude); 
        dtostrf(-999.99, WIDTH_ALTITUDE, PRECISION_ALTITUDE, bufferAltitude);
      }
      //sprintf(bufferLat, "%f", gnss.location.lat());
      
      dtostrf(display.getAGL(), WIDTH_ALTITUDE, PRECISION_ALTITUDE, bufferAGL);      


      dtostrf(temp.temperature, WIDTH_TEMP, PRECISION_TEMP_HUMIDITY, bufferTemp);
      dtostrf(humidity.relative_humidity, WIDTH_HUMIDITY, PRECISION_TEMP_HUMIDITY, bufferHumidity);

      //sprintf(buffer, "temperature:%f humidity:%f location:(%lf,%s,%lf)", temp.temperature, humidity.relative_humidity, gnss.location.lat(), bufferLong, gnss.altitude.meters());
      sprintf(buffer, "temperature:%s humidity:%s latitude:%s longitude:%s altitude:%.2lf agl:%.2lf", bufferTemp, bufferHumidity, bufferLat, bufferLong, gnss.altitude.meters(), display.getAGL());
      //sprintf(buffer, "temperature:%f humidity:%f", temp.temperature, humidity.relative_humidity);

      // This seems to crash the chip
      radioError = radio.sendMsg(buffer);

  }

}

