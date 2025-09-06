//
// L O R A . H
//

#ifndef RYLR988_H
#define RYLR988_H

#include "Arduino.h"


class LoraConfiguration
{
public:
    int address;
    int baud;
    int network;
};


class RYLR988
{
public:
    // Error codes
    enum SERIAL_ERR
    {
        ERR_NO_ERROR,
        ERR_BUF_EXCEEDED,
        ERR_READ_REQ,
        ERR_NO_RADIO,
        ERR_TIMEOUT,
        ERR_NO_RX,
        ERR_RADIO_NOT_READY,
        ERR_INCOMPLETE,
        ERR_TX,
        ERR_INVALID
    };

    RYLR988(HardwareSerial *serial, int8_t rx, int8_t tx);
    SERIAL_ERR begin(uint32_t baud, u16_t address, u16_t network);
    LoraConfiguration getLoraConfiguration();
    int setLoraConfiguration(LoraConfiguration cfg);
    SERIAL_ERR ping();
    SERIAL_ERR sendMsg(const char* msg);
    SERIAL_ERR waitMsg(char* msg, u16_t bufferSize, int timeout);
    SERIAL_ERR waitForServerOK(u16_t timeout);
    String errorString(SERIAL_ERR err);
    void debug();

    void setDestination(int i);
    int getDestination();
    char *getUID();
    uint16_t getAddress();

    SERIAL_ERR lastError;
    char lastMessage[80];
    
private:
    // Doesn't seem to be a great way to determine the number of enums, so this is hard-coded
    String errorStrings[9] = {
        "No Error",
        "Read/Write Buffer exeeded",
        "Read message before writing",
        "No radio found",
        "Timeout",
        "No data to receive",
        "Radio not ready",
        "Incomplete message read"
        "Invalid configuration"
    };


    bool commandOK(unsigned long timeout);
    String lastResponse;

    LoraConfiguration cfg;
    String extractActualMsg(char *msg);
    void sendDebugMessage(const char* msg);
    
    bool begun;

    // The destination radio
    int destination;
    // The UID of this radio
    char uid[32];
    // The address of this radio
    uint16_t address;

    // Serial communication
    int8_t pinTX;
    int8_t pinRX;


    // The serial device used to communicate with radio
    HardwareSerial *serialDevice;


};

#endif