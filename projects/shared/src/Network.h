//
// N E T W O R K 
//
// A wrapper around the arduino wifi support

#include <Arduino.h>

#ifndef _incNetwork
#define _incNetwork

#include "WiFi.h"

class Network {
    Network();

    bool begin(char *ssid, char *passwword, int attempts);
    void scan();
    
    private:
    char *ssid;
    bool connected;


}

#endif