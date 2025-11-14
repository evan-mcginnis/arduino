//
// O L E D . H
//

// ETL Items
// #define ETL_NO_STL
// #include <etl/vector.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <CircularBuffer.hpp>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels


#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define MAX_READINGS 5

// Thresholds for the standard deviation to determine if the fix is goood

#define THRESHOLD_ALTITUDE  1.0
#define THRESHOLD_LATITUDE  0.001
#define THRESHOLD_LONGITUDE 0.001


enum class POSITION {
    latitude = 0,
    longitude = 1,
    altitude = 2
};

class location {
    public:
        location(double lat, double lng, double alt);
        double latitude;
        double longitude;
        double altitude;
};

enum class SELECTION {
    gnss = 0,
    armed = 1
};

class OLED {
    public:
        OLED();
        ~OLED();
        void initialize();
        void splash();

        Adafruit_SSD1306 *screen();

        void debug(const char* msg);
        
        void updateGNSSStatus(bool status, char *msg);
        void updateLocation(double_t latitude, double_t longitude);
        void updateAccuracy(double_t hdop, double_t vdop);

        void updateAltitude(double altitude);

        void updateSatellites(uint32_t satellites);

        void updateDateTime(const char* dateOfObservation, const char* timeOfObservation);
        void updateDate(uint16_t year, uint8_t month, uint8_t day);
        void updateTime(uint8_t hour, uint8_t minute, uint8_t second);

        void forceDisplay(SELECTION displaySelection);
        void displaySelected(SELECTION displaySelection);
        void displayLocation();
        void displayArmed();

        double getLatitude();
        double getLongitude();
        double getAltitude();
        double getAGL();

        void calculateDelta(POSITION readingType, double value);

        double highestAltitude;

        double stdDeviationAltitude;
        double stdDeviationLatitude;
        double stdDeviationLongitude;

        // The last observation
        double lastLatitude;
        double lastLongitude;
        double lastAltitude;
        char lastDate[32];
        char lastTime[32];

        void markLocation();
        location * getMarkedLocation();

        void swapArmedMode();

        void swapDebugMode();

        void lowPowerMode(bool lowPower);

        void setErrorMessage(const char* errorMessage);

        bool getDebugMode();

    private:
        Adafruit_SSD1306 *display;
        char buffer[80];

        double latitude;
        double longitude;
        double hdop;
        double vdop;

        double altitude;
        uint32_t satellites;

        double groundAltitude;
        double AGL;

        bool locationDirty;
        bool force;
        bool error;

        char dateOfObservation[32];
        char timeOfObservation[32];

        location *markedLocation;

        double markedLatitude;
        double markedLogitude;
        double markedAltitude;



        // Keep the deltas of the prior observations
        CircularBuffer<double, MAX_READINGS> latitudes;
        CircularBuffer<double, MAX_READINGS> longitudes;
        CircularBuffer<double, MAX_READINGS> altitudes;
 
        bool lowPower;
        bool armed;
        bool debugMode;

        bool ok;
        char *statusMessage;

        double stdDeviation(double *readings, uint8_t length);

    

};