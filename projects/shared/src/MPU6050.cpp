#include "MPU6050.h"
#include <Wire.h>

MPU6050::MPU6050(int address)
{
    this->address = address;
    this->begun = false;
}

void
MPU6050::begin()
{
    Wire.begin();
    Wire.setClock(25000);
    Wire.beginTransmission(this->address);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
    this->begun = true;
}

bool
MPU6050::read()
{
    int rc = 0;
    int bytesReturned = 0;

    if (!begun) {
        return false;
    }

    Wire.beginTransmission(this->address);
    Wire.write(0x3B);
    rc = Wire.endTransmission(false);
    if (rc != 0) {
        Serial.printf("Failure code %d encountered in endTransmission()\n", rc);
        return false;
    }
    bytesReturned = Wire.requestFrom(this->address, 7 * 2, true);
    if (bytesReturned == 7 * 2) {

        this->accelX = Wire.read()<<8 | Wire.read(); // 0x3B and 0x3C
        this->accelY = Wire.read()<<8 | Wire.read(); // 0x3D and 0x3E
        this->accelZ = Wire.read()<<8 | Wire.read(); // 0x3F and 0x40
        this->temperature = Wire.read()<<8 | Wire.read(); // 0x41 and 0x42
        this->gyroX = Wire.read()<<8 | Wire.read(); // 0x43 and 0x44
        this->gyroY = Wire.read()<<8 | Wire.read(); // 0x45 and 0x46
        this->gyroZ = Wire.read()<<8 | Wire.read(); // 0x47 and 0x48

        delay(100);
    }
    else {
        Serial.printf("---- Error in requestFrom() %d bytes available ----\n", bytesReturned);
    }

    return true;
}