//
// M P U 6 0 5 0
//
// Gryo and accelerometer

#ifndef _HPU6050
#define _MPU6050

#include "Arduino.h"

class MPU6050 {
    public:
        MPU6050(int address);
        void begin();
        bool read();

        // Values to be read from MPU
        int16_t accelX;
        int16_t accelY;
        int16_t accelZ;
        int16_t gyroX;
        int16_t gyroY;
        int16_t gyroZ;
        int16_t temperature;

    private:
        int address;
        bool begun;

};

#endif
