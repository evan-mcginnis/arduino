//
// CALIBRATION
// Evan McGinnis
// 9 Aug 2025
//
#ifndef CALIBRATION_h
#define CALIBRATION_h

#include "Arduino.h"

class MoistureSensorCalibration {
    public:
        MoistureSensorCalibration();
        void read();        
        void write();
        void setWetPoint(int point);
        void setDryPoint(int point);
        int getWetPoint();
        int getDryPoint();

        int length();

    private:
        int wetPoint;
        int dryPoint;

        int addressWet = 0;
        int addressDry = sizeof(int);

};

#endif
