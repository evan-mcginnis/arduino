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

enum class POSITION {
    latitude = 0,
    longitude = 1,
    altitude = 2
};

class OLED {
    public:
        OLED();
        void initialize();
        void splash();

        Adafruit_SSD1306 *screen();

        void debug(const char* msg);
        
        void updateLocation(double_t latitude, double_t longitude, bool isValid);

        void updateAltitude(double altitude);

        void updateSatellites(uint32_t satellites);

        void updateDateTime(const char* dateOfObservation, const char* timeOfObservation);
        void updateDate(uint16_t year, uint8_t month, uint8_t day);
        void updateTime(uint8_t hour, uint8_t minute, uint8_t second);


        void displayLocation();

        double getLatitude();
        double getLongitude();
        double getAltitude();

        void calculateDelta(POSITION readingType, double value);

        double stdDeviationAltitude;
        double stdDeviationLatitude;
        double stdDeviationLongitude;

    private:
        Adafruit_SSD1306 *display;
        char buffer[80];

        double latitude;
        double longitude;
        bool fixIsGood;

        double altitude;
        uint32_t satellites;

        bool locationDirty;

        char dateOfObservation[32];
        char timeOfObservation[32];

        // The last observation
        double lastLatitude;
        double lastLongitude;
        double lastAltitude;
        char lastDate[32];
        char lastTime[32];

        // Keep the deltas of the prior observations
        CircularBuffer<double, MAX_READINGS> latitudes;
        CircularBuffer<double, MAX_READINGS> longitudes;
        CircularBuffer<double, MAX_READINGS> altitudes;


        double stdDeviation(double *readings, u8_t length);




};