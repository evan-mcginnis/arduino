//
// O L E D . C P P
//

#include "OLED.h"

#include <CircularBuffer.hpp>
#include <algorithm>



/// @brief OLED Constructor.  Only allocates.
OLED::OLED()
{
    display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    latitude = 0.0;
    longitude = 0.0;
    hdop = 100;
    altitude = 0.0;
    highestAltitude = 0.0;
    satellites = 0;
    strcpy(lastDate, "");
    strcpy(lastTime, "");

    markedLocation = new location(0.0, 0.0, 0.0);
    lowPower = false;

    groundAltitude = 0.0;

    locationDirty = false;
    force = false;

    armed = false;

    ok = false;
}

OLED::~OLED()
{
    delete markedLocation;
}

/// @brief Initislize the display. Will block until screen is started.
void OLED::initialize()
{
    if(!display->begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        //Serial1.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }
    display->setTextSize(1);
    display->setTextColor(WHITE);
}

// Goat Logo
const unsigned char logo[] = {
	// 'Screenshot 2025-10-09 155846, 46x31px
	0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xfc, 0x00, 
	0x1f, 0xfc, 0xff, 0xff, 0xe0, 0x00, 0x07, 0xfc, 0xff, 0xff, 0x80, 0x00, 0x01, 0xfc, 0xff, 0xfe, 
	0x00, 0x00, 0x00, 0xfc, 0xff, 0xfc, 0x00, 0x1f, 0x80, 0x7c, 0xff, 0xf8, 0x00, 0xff, 0xf0, 0x3c, 
	0xff, 0xe0, 0x03, 0xff, 0xfc, 0x1c, 0xff, 0xc0, 0x07, 0xff, 0xff, 0x1c, 0xff, 0x80, 0x1f, 0xff, 
	0xff, 0x8c, 0xff, 0x00, 0xbf, 0xff, 0xff, 0xcc, 0xfe, 0x00, 0x7f, 0xff, 0xff, 0xc4, 0xfc, 0x00, 
	0x3f, 0xff, 0xff, 0xe4, 0xfc, 0x00, 0x1f, 0xff, 0xff, 0xf4, 0xf8, 0x00, 0x0f, 0xff, 0xff, 0xf4, 
	0xf2, 0x00, 0x07, 0xff, 0xff, 0xf4, 0xe0, 0x00, 0x3f, 0xff, 0xff, 0xfc, 0xc0, 0x00, 0x1f, 0xff, 
	0xff, 0xfc, 0xc0, 0x00, 0x1f, 0xff, 0xff, 0xfc, 0x80, 0x00, 0x0f, 0xff, 0xff, 0xfc, 0x00, 0x00, 
	0x07, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x07, 0xff, 0xff, 0xfc, 0x00, 0x20, 0x03, 0xff, 0xff, 0xfc, 
	0x83, 0xc0, 0x03, 0xff, 0xff, 0xfc, 0xe3, 0xc0, 0x01, 0xff, 0xff, 0xfc, 0xf3, 0xc0, 0x01, 0xff, 
	0xff, 0xfc, 0xff, 0xc0, 0x00, 0xff, 0xff, 0xfc, 0xff, 0xc0, 0x00, 0xff, 0xff, 0xfc, 0xff, 0x80, 
	0x00, 0xff, 0xff, 0xfc, 0xff, 0x80, 0x00, 0x7f, 0xff, 0xfc
};
#define LOGO_WIDTH 46
#define LOGO_HEIGHT 31



/// @brief Display a welcome message
void OLED::splash()
{
    display->clearDisplay();
    display->display();
    display->fillScreen(BLACK);
    // Draw the logo centered on screen
    display->drawBitmap((SCREEN_WIDTH / 2) - (LOGO_WIDTH / 2), 0, logo, LOGO_WIDTH, LOGO_HEIGHT, WHITE);
    //display->printf("Welcome");
    display->display();
    delay(3000);
    display->clearDisplay();
    display->display();
}

/// @brief The screen object so direct adafruit calls can be made
/// @return pointer to the adafruit object
Adafruit_SSD1306 * OLED::screen()
{
    return display;
}

void
OLED::debug(const char* msg)
{
    display->print(msg);
    display->display();

}

void OLED::updateGNSSStatus(bool status, char *msg)
{
    this->ok = status;
    this->statusMessage = msg;
}

/// @brief Update the latitude and longitude
/// @param latitude
/// @param longitude
void OLED::updateLocation(double_t latitude, double_t longitude)
{

    if (!this-locationDirty) {
        this->locationDirty = latitude != lastLatitude || longitude != lastLongitude;
    }
    // The previous readings
    lastLatitude = this->latitude;
    lastLongitude = this->longitude;

    // The current readings
    this->latitude = latitude;
    this->longitude = longitude;
}

void OLED::updateAccuracy(double_t hdop, double_t vdop)
{
    this->hdop = hdop;
    this->vdop = vdop;
}

/// @brief Update the altitude with the current reading
/// @param altitude The altitude reading
void OLED::updateAltitude(double altitude)
{
    if (!this->locationDirty){
        this->locationDirty = altitude != lastAltitude;
    }
    AGL = altitude - groundAltitude;

    // The previous readings
    lastAltitude = this->altitude;

    highestAltitude = std::max(this->altitude, altitude);

    // The current reading
    this->altitude = altitude;
}

/// @brief Update the satellite count
/// @param satellites 
void OLED::updateSatellites(uint32_t satellites)
{
    if (!this->locationDirty) { 
        this->locationDirty = (this->satellites != satellites);
    }
    this->satellites = satellites;
}

/// @brief Update the date and time of the observation
/// @param dateOfObservation 
/// @param timeOfObservation 
void OLED::updateDateTime(const char *dateOfObservation, const char *timeOfObservation)
{
    strncpy(this->dateOfObservation, dateOfObservation, sizeof(this->dateOfObservation));
    strncpy(this->timeOfObservation, timeOfObservation, sizeof(this->timeOfObservation));
}

void OLED::updateDate(uint16_t year, uint8_t month, uint8_t day)
{
    sprintf(dateOfObservation, "%d/%02d/%02d", year, month, day);
}

void OLED::updateTime(uint8_t hour, uint8_t minute, uint8_t second)
{
    sprintf(timeOfObservation, "%02d:%02d:%02d", hour, minute, second);
}

#define WIDTH_LAT           7
#define WIDTH_LONG          9
#define WIDTH_ALTITUDE      5
#define PRECISION_LAT_LONG  5
#define PRECISION_ALTITUDE  2
#define WIDTH_TEMP          5
#define WIDTH_HUMIDITY      5
#define PRECISION_TEMP_HUMIDITY 3

/// @brief Force the display of the specified selection, regardless of change
/// @param displaySelection 
void OLED::forceDisplay(SELECTION displaySelection)
{
    if (displaySelection == SELECTION::gnss) {
        force = true;
        display->clearDisplay();
    }
}

/// @brief Display the specified information on the current screen
/// @param displaySelection 
void OLED::displaySelected(SELECTION displaySelection)
{
    switch (displaySelection) {
        case SELECTION::gnss:
            displayLocation();
        break;
        case SELECTION::armed:
            displayArmed();
        break;
    }
        
}

/// @brief Update display if required with current information
void OLED::displayLocation()
{
    char tmpLatitude[12];
    char tmpLongitude[12];
    char tmpAltitude[12];

    // Don't update screen in low power mode
    if (lowPower) {
        return;
    }

    // Check to see if there is an error in the fix
    if (!this->ok) {
        //display->clearDisplay();
        display->setCursor(0,0);
        display->invertDisplay(true);
        display->setTextSize(1);
        display->println(this->statusMessage);
        display->display();
        return;
    }

    display->setTextSize(1);

    // If the std deviation is too high, invert the display to indicate this
    if (stdDeviationAltitude > THRESHOLD_ALTITUDE ||
        stdDeviationLatitude > THRESHOLD_LATITUDE ||
        stdDeviationLongitude > THRESHOLD_LONGITUDE) {
            display->invertDisplay(true);
    }
    else {
        display->invertDisplay(false);
    }


    // Check if the location has changed or if the display is forced
    if (locationDirty || force) {
        force = false;
        display->clearDisplay();
        display->display();
        display->setTextSize(1);
        display->setCursor(0,0);
        
        dtostrf(latitude, WIDTH_LAT, PRECISION_LAT_LONG, tmpLatitude);  
        dtostrf(longitude, WIDTH_LONG, PRECISION_LAT_LONG, tmpLongitude);
        dtostrf(altitude, WIDTH_ALTITUDE, PRECISION_ALTITUDE, tmpAltitude);

        sprintf(buffer, "Satellites: %d Mode: %s", satellites, this->debugMode ? "D" : "N");
        display->print(buffer);

        if (this->debugMode) {
            display->setCursor(160, 0);
            display->print("D");
        }

        display->setCursor(0, 10);
        sprintf(buffer,"Lat: %s\nLong: %s\n", tmpLatitude, tmpLongitude);
        display->print(buffer);
        sprintf(buffer, "Alt: %sm\n", tmpAltitude);
        display->print(buffer);
        sprintf(buffer, "AGL: %.02lfm\n", AGL);
        display->print(buffer);
        sprintf(buffer, "HDOP: %.02lf VDOP: %0.2lf", hdop, vdop);
        display->print(buffer);
        display->display();
    }

    // Update the time/date of the last acquisition

    // There are 5 lines of text above, each 8 pixels high
    // Overwrite the last display date/time in black to erase it.
    if (strlen(lastDate) > 0 ) {
        display->setCursor(0, 7 * 8);
        display->setTextColor(BLACK);
        display->print(lastDate);
        display->print(" ");
        display->print(lastTime);
        delay(100);
    }


    display->setCursor(0, 7 * 8);
    display->setTextColor(WHITE);
    display->print(dateOfObservation);
    display->print(" ");
    display->print(timeOfObservation);
    display->display();
    delay(100);
    
    strcpy(lastTime, timeOfObservation);
    strcpy(lastDate, dateOfObservation);
}

/// @brief Display the arming state
void OLED::displayArmed()
{
    display->clearDisplay();
    display->display();
    display->setTextSize(2);
    display->setCursor(0,0);
    display->println(this->armed ? "ARMED" : "WAITING");
    display->println(this->debugMode ? "DEBUG" : "FIELD");
    display->display();
}

/// @brief Get the current recorded longitude
/// @return Longitude in decimal degrees
double OLED::getLongitude()
{
    return this->longitude;
}

/// @brief Get the current recorded lantitude
/// @return Latitude in decimal degrees
double OLED::getLatitude()
{
    return this->longitude;
}

/// @brief Get the current recorded altitude
/// @return Altititude in meters
double OLED::getAltitude()
{
    return this->altitude;
}

/// @brief Get the current calculated distance AGL
/// @return AGL in meters
double OLED::getAGL()
{
    return AGL;
}

/// @brief Calculate the standard deviations of the last N readings
/// @param readingType Type of reading (latitude, longitude, altitude)
/// @param value Reading value
void OLED::calculateDelta(POSITION readingType, double value)
{
    double readings[MAX_READINGS];

    switch (readingType) {
        case POSITION::altitude:
            this->altitudes.push(value);
            altitudes.copyToArray(readings);
            stdDeviationAltitude = stdDeviation(readings, this->altitudes.size());
        break;
        case POSITION::latitude:
            this->latitudes.push(value);
            latitudes.copyToArray(readings);
            stdDeviationLatitude = stdDeviation(readings, this->latitudes.size());
        break;
        case POSITION::longitude:
            this->longitudes.push(value);
            longitudes.copyToArray(readings);
            stdDeviationLongitude = stdDeviation(readings, this->longitudes.size());
        break;
    }

}

/// @brief Mark the current location as the waypoint
void OLED::markLocation()
{
    markedLocation->altitude = this->altitude;
    markedLocation->longitude = this->longitude;
    markedLocation->latitude = this->latitude;
}

/// @brief Get the marked location
/// @return Location with lat,long,alt
location * OLED::getMarkedLocation()
{
    return markedLocation;
}


/// @brief Invert the current armed/waiting mode.
void OLED::swapArmedMode()
{
    // In armed mode, the ground level is set to the current altitude
    this->armed = !this->armed;
    if (this->armed) {
        AGL = 0;
        groundAltitude = altitude;
    }
}

void OLED::swapDebugMode()
{
    this->debugMode = !this->debugMode;
}

/// @brief Enable low power mode. Low power mode will turn off the display to conserve battery.
/// @param lowPower 
void OLED::lowPowerMode(bool lowPower)
{
    this->lowPower = lowPower;

    if (lowPower) {
        display->clearDisplay();
    }

}

void OLED::setErrorMessage(const char *errorMessage)
{
}

bool OLED::getDebugMode()
{
    return this->debugMode;
}

double OLED::stdDeviation(double *readings, uint8_t length)
{
    double mean = 0.0;
    double sum = 0.0;
    double working[length];

    // Overall mean
    for (int i = 0; i < length; i++) {
        sum += readings[i];
    }
    mean = sum / length;
    
    // Square the difference of each data point from mean
    // and find the sum
    sum = 0.0;
    for (int i = 0; i < length; i++) {
        sum += std::pow(std::abs(readings[i] - mean), 2);
    }

    // Determine the variance
    double variance = sum / length;

    // Take the square root
    return std::sqrt(variance);
}

location::location(double lat, double lng, double alt)
{
    this->latitude = lat;
    this->longitude = lng;
    this->altitude = alt;
}
