// 
// D R O N E
//
// This will report position every second
//

#include <Arduino.h>

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



// See this for using multiple UARTS
// https://forum.seeedstudio.com/t/how-to-use-serial1-with-xiao-esp32c3/266306/4

//Define two Serial devices mapped to the two internal UARTs
HardwareSerial MySerial0(0);
HardwareSerial MySerial1(1);

// The radio will use TX/RX ports on the XIAO -- D6/D7
// Indicate this with -1 and -1
RYLR988::SERIAL_ERR radioError;
RYLR988 radio(& MySerial0, -1, -1);

// The GNSS will use TX/RX on XIAO D1/D2
#define PIN_GNSS_RX 9
#define PIN_GNSS_TX 10
#define BAUD_GNSS 9600

// Message to send through radio or on OLED
char buffer[80];

void setup() {
  // C O N S O L E
  Serial.begin(115200);
  delay(100);


  // O L E D
  delay(250);
  display.initialize();
  display.splash();

  // Sensor
  if (! aht.begin()) {
    Serial.println("Could not find AHT10");
    while (1) delay(10);
  }
  display.screen()->setCursor(0,0);
  display.debug("AHT10 found");
  delay(1000);
  display.screen()->clearDisplay();

  // R A D I O
  radioError = radio.begin(RADIO_BAUD, RADIO_ID, RADIO_NETWORK);

  display.screen()->setCursor(0,0);
  if (radioError == RYLR988::ERR_NO_ERROR) {
    sprintf(buffer, "Radio ID: %d", radio.getAddress());
    display.debug(buffer);
    delay(1000);
  } else {
    display.debug("Radio Init Error");
  }

  // GNSS
  MySerial1.begin(9600, SERIAL_8N1, PIN_GNSS_RX, PIN_GNSS_TX);
  //gpsSerial.begin(BAUD_GNSS);
  display.screen()->setCursor(0, 0);
  display.screen()->clearDisplay();
  display.debug("GPS setup");
  delay(750);

  display.screen()->setCursor(0, 0);
  display.screen()->clearDisplay();
  display.screen()->display();

}




void loop() {
  sensors_event_t humidity, temp;
  char debugBuffer[80];

  // GNSS
  if (MySerial1.available() == 0) {
    Serial.println("No Location Data");
  }

  while (MySerial1.available()) {
    char token = MySerial1.read();
    gnss.encode(token);
    // Debug
    //Serial.printf("%c", token);
  }

  // sprintf(debugBuffer, "Location Fix: %s", gnss.location.isValid() ? "Good" : "Bad");
  // display.screen()->setCursor(0, 0);
  // display.debug(debugBuffer);
  
  if (gnss.location.isUpdated()) {
    Serial.print("Latitude: ");
    Serial.println(gnss.location.lat(), 6);
    Serial.print("Longitude: ");
    Serial.println(gnss.location.lng(), 6);
    if (gnss.satellites.isValid()) {
      Serial.print("Satellites: ");
      Serial.println(gnss.satellites.value());
    }
    if (gnss.altitude.isValid()) {
      Serial.print("Altitude: ");
      Serial.println(gnss.altitude.meters());
    }


    Serial.printf("%d/%02d/%02d %d:%d:%d\n", gnss.date.year(), gnss.date.month(), gnss.date.day(), gnss.time.hour(), gnss.time.minute(), gnss.time.second());
    display.updateDate(gnss.date.year(), gnss.date.month(), gnss.date.day());
    display.updateTime(gnss.time.hour(), gnss.time.minute(), gnss.time.second());
    
    // Calculate how stable the readings are
    display.calculateDelta(POSITION::latitude, gnss.location.lat());
    display.calculateDelta(POSITION::longitude, gnss.location.lng());
    display.calculateDelta(POSITION::altitude, gnss.altitude.meters());

    Serial.printf("Std Deviations: latitude (%lf) longitude (%lf) altitude (%lf)\n", display.stdDeviationLatitude, display.stdDeviationLongitude, display.stdDeviationAltitude);

    // display.screen()->setCursor(0, 0);
    // display.screen()->clearDisplay();
    // display.screen()->display();
    display.updateLocation(gnss.location.lat(), gnss.location.lng(), gnss.location.isValid());
    display.updateAltitude(gnss.altitude.meters());
    display.updateSatellites(gnss.satellites.value());
    display.displayLocation();
  }



  aht.getEvent(&humidity, &temp);

  //sprintf(buffer, "temperature:%f humidity:%f location:(%lf,%lf)", temp.temperature, humidity.relative_humidity, display.getLatitude(), display.getLongitude());
  sprintf(buffer, "temperature:%f humidity:%f", temp.temperature, humidity.relative_humidity);
  radioError = radio.sendMsg(buffer);
}

