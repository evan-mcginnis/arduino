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
    };

    RYLR988();
    SERIAL_ERR begin(uint32_t baud, u16 address, u16 network);
    LoraConfiguration getLoraConfiguration();
    int setLoraConfiguration(LoraConfiguration cfg);
    SERIAL_ERR ping();
    SERIAL_ERR sendMsg(const char* msg);
    SERIAL_ERR waitMsg(char* msg, u16 bufferSize, int timeout);
    String errorString(SERIAL_ERR err);
    void debug();
    char lastMessage[80];
    void setDestination(int i);
    int getDestination();

private:
    // Doesn't seem to be a great way to determine the number of enums, so this is hard-coded
    String errorStrings[8] = {
        "No Error",
        "Read/Write Buffer exeeded",
        "Read message before writing",
        "No radio found",
        "Timeout",
        "No data to receive",
        "Radio not ready",
        "Incomplete message read"
    };


    LoraConfiguration cfg;
    String extractActualMsg(char *msg);
    SERIAL_ERR sendMessage(char *msg);
    
    // The destination radio
    int destination;
};

#endif