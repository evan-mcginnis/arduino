#include "MoistureSensorCalibration.h"

#include <EEPROM.h>


MoistureSensorCalibration::MoistureSensorCalibration()
{
    wetPoint = 780;
    dryPoint = 0;
}

void MoistureSensorCalibration::read()
{
    EEPROM.get(0, wetPoint);
    EEPROM.get(sizeof(int), dryPoint);
}

void MoistureSensorCalibration::write()
{
    EEPROM.put(0, wetPoint);
    EEPROM.put(sizeof(int), dryPoint);
}

void MoistureSensorCalibration::setWetPoint(int point)
{
    wetPoint = point;
}


void MoistureSensorCalibration::setDryPoint(int point)
{
    dryPoint = point;
}

int MoistureSensorCalibration::getWetPoint()
{
    return wetPoint;
}

int MoistureSensorCalibration::getDryPoint()
{
    return dryPoint;
}

int MoistureSensorCalibration::length()
{
    uint16_t size = EEPROM.length();

    return(size);
}
