//
// O L E D . C P P
//

#include "OLED.h"

#include <CircularBuffer.hpp>



/// @brief OLED Constructor.  Only allocates.
OLED::OLED()
{
    display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    latitude = 0.0;
    longitude = 0.0;
    fixIsGood = false;
    altitude = 0.0;
    satellites = 0;
    strcpy(lastDate, "");
    strcpy(lastTime, "");

    locationDirty = false;
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

/// @brief Display a welcome message
void OLED::splash()
{
    display->clearDisplay();
    display->display();
    display->print("Welcome");
    display->display();
    delay(1000);
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

void OLED::updateLocation(double_t latitude, double_t longitude, bool isValid)
{

    this->locationDirty = latitude != lastLatitude || longitude != lastLongitude;

    // The previous readings
    lastLatitude = this->latitude;
    lastLongitude = this->longitude;

    // The current readings
    this->latitude = latitude;
    this->longitude = longitude;
    this->fixIsGood = isValid;
}

void OLED::updateAltitude(double altitude)
{
    this->locationDirty = altitude != lastAltitude;

    // The previous readings
    lastAltitude = this->altitude;

    // The current reading
    this->altitude = altitude;
}

void OLED::updateSatellites(uint32_t satellites)
{
    this->locationDirty = satellites != this->satellites;
    this->satellites = satellites;
}

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

/// @brief Update display if required
void OLED::displayLocation()
{
    display->setTextSize(1);

    // Check if the location has changed
    if (locationDirty) {
        display->clearDisplay();
        display->display();
        display->setTextSize(1);
        display->setCursor(0,0);
        sprintf(buffer, "Satellites: %d", satellites);
        display->print(buffer);
        display->setCursor(0, 10);
        sprintf(buffer,"Lat: %lf\nLong: %lf\n", latitude, longitude);
        display->print(buffer);
        sprintf(buffer, "Alt: %.2lfm\n", altitude);
        display->print(buffer);
        display->display();
    }

    // Update the time/date of the last acquisition

    // There are 4 lines of text above, each 8 pixels high
    // Overwrite the last display in black to erase it.
    if (strlen(lastDate) > 0 ) {
        display->setCursor(0, 5 * 8);
        display->setTextColor(BLACK);
        display->print(lastDate);
        display->print(" ");
        display->print(lastTime);
        delay(100);
    }


    display->setCursor(0, 5 * 8);
    display->setTextColor(WHITE);
    display->print(dateOfObservation);
    display->print(" ");
    display->print(timeOfObservation);
    display->display();
    delay(100);
    
    strcpy(lastTime, timeOfObservation);
    strcpy(lastDate, dateOfObservation);
}

double OLED::getLongitude()
{
    return this->longitude;
}

double OLED::getLatitude()
{
    return this->longitude;
}

double OLED::getAltitude()
{
    return this->altitude;
}

/// @brief Calculate the average of deltas in the last N readings
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
            stdDeviationLatitude = stdDeviationAltitude = stdDeviation(readings, this->latitudes.size());
        break;
        case POSITION::longitude:
            this->longitudes.push(value);
            longitudes.copyToArray(readings);
            stdDeviationLongitude = stdDeviation(readings, this->longitudes.size());
        break;
    }

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


