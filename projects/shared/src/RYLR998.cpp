//
// R L R 9 9 8 . C P P
//
// RLRY998 Support
//

#include "RYLR998.h"
#include "Arduino.h"

// Messages that can be sent to or emiited from the radio

#define MSG_OK          "+OK"
#define MSG_MODE        "+MODE"
#define MSG_IPR         "+IPR"
#define MSG_BAND        "+BAND"
#define MSG_PARAMETER   "+PARAMETER"
#define MSG_ADDRESS     "+ADDRESS"
#define MSG_NETWORK     "+NETWORKID"
#define MSG_CPIN        "+CPIN"
#define MSG_CRFOP       "+CRFFOP"
#define MSG_SEND        "+SEND"
#define MSG_RCV         "+RCV"
#define MSG_UID         "+UID"
#define MSG_VER         "+VER"
#define MSG_FACTORY     "+FACTORY"
#define MSG_RESET       "+RESET"
#define MSG_READY       "+READY"
#define MSG_ERR         "+ERR"
#define MSK_OK          "+OK"


// This message is a bit odd. It takes the form AT+FCCT=[0,1,2]
#define MSG_FCCT    "AT+FCCT"

#define MSG_PREFIX  "AT"

#define SEPARATOR   "="

#define QUERY       "?"

// Pre-defined messages expected from server
#define MSG_PROCEED     "OK"
#define MSG_DUPLICATE   "DUPLICATE"

#define SELECT_ADDRESS  0
#define MIN_ADDRESS     2
#define MAX_ADDRESS     1024

// The timeout for local chip operations
#define TIMEOUT 1000

// The timeout for responses from server
#define SERVER_TIMEOUT 4000



/// @brief Constructor for radio object
/// @param serialPort serial port to use for communication
/// @param rx receive pin number
/// @param tx transmit pin number
RYLR988::RYLR988(HardwareSerial *serialPort, int8_t rx, int8_t tx)
{
    // The default radio destination
    destination = 1;
    serialDevice = serialPort;
    uid[0] = 0;

    pinTX = -1;
    pinRX = -1;
}

/// @brief Set up the radio
/// @param baud The baud -- 115200 is commonly used for the RYLT998
/// @param address  The address of this radio -- 0 indicates an address will be chosen
/// @param network The network
/// @return ERR_NO_ERR on success
RYLR988::SERIAL_ERR RYLR988::begin(uint32_t baud, u16_t address, u16_t network)
{
    char configurationMsg[80];
    char helloMessage[80];
    SERIAL_ERR err;
    String rsp;
    bool waitForOKFromServer = false;

    char *radioID;

    serialDevice->begin(baud, SERIAL_8N1, pinRX, pinTX);

    // Check to see if the chip is ready to accept commands
    err = ping();

    if (err == ERR_NO_ERROR) {

        // If a random address is chosen, the server gives the final OK
        if (address == SELECT_ADDRESS) {
                randomSeed(analogRead(0));
                address = random(MIN_ADDRESS, MAX_ADDRESS);
                waitForOKFromServer = true;
        }
        // Configure the address
        sprintf(configurationMsg, "%s%s=%d", MSG_PREFIX, MSG_ADDRESS, address);
        serialDevice->println(String(configurationMsg));

        
        if (!commandOK(TIMEOUT))
            return (lastError);
        
        //sendDebugMessage("Address OK");
        

        this->address = address;

        // Configure the network
        sprintf(configurationMsg, "%s%s=%d", MSG_PREFIX, MSG_NETWORK, network);
        serialDevice->println(String(configurationMsg));
        //rsp = Serial.readString();

        if (!commandOK(TIMEOUT))
            return (lastError);    

        //sendDebugMessage("Network OK");

        radioID = getUID();
        //radioID = uid;
        // Announce this system
        sprintf(helloMessage, "start:%s", radioID);
        sprintf(configurationMsg, "%s%s=%d,%d,%s", MSG_PREFIX, MSG_SEND,  destination, strlen(helloMessage), helloMessage);
        serialDevice->println(String(configurationMsg));

        if (!commandOK(TIMEOUT))
            return (lastError);
        //rsp = serialDevice->readString();

        if (waitForOKFromServer){
            // The server will send an ack for the startup.
            err = waitForServerOK(SERVER_TIMEOUT);
        }
    }

    return (err);
    
}

/// @brief Get the configuration of the radio. NOT IMPLEMENTED
/// @return 
LoraConfiguration RYLR988::getLoraConfiguration()
{
    return LoraConfiguration();
}

/// @brief Set the configuration of the radio. NOT IMPLEMENTED
/// @param cfg 
/// @return 
int RYLR988::setLoraConfiguration(LoraConfiguration cfg)
{
    return 0;
}

/// @brief Ping the radio to make sure it is ready for commands
/// @return ERR_NO_ERROR if it is OK, ERR_NO_RADIO or ERR_RADIO_NOT_READY if not
RYLR988::SERIAL_ERR RYLR988::ping()
{
    SERIAL_ERR err = ERR_NO_ERROR;
    String rsp;

    // At device startup, it emits +READY
    if (serialDevice->available()) {
        rsp = serialDevice->readString();
    }

    // The device responds with +OK when presented with AT
    serialDevice->println(MSG_PREFIX);

    err = commandOK(TIMEOUT) ? ERR_NO_ERROR : ERR_RADIO_NOT_READY;

    return err;

}

/// @brief Send the message specified
/// @param msg the message
/// @return Result of send operation. ERR_NO_ERR indicates successful send.
RYLR988::SERIAL_ERR RYLR988::sendMsg(const char * msg)
{
    SERIAL_ERR err = ERR_NO_ERROR;
    size_t written = 0;
    char rylr988Message[80];

    // The IDE doesn't count the else condition as use, so I need this hack to 
    // shut it up. 
    (void)written;

    //If bytes are waiting tp be read, just return
    if (serialDevice->available() > 0){
        err = SERIAL_ERR::ERR_READ_REQ;
    }
    // If the message exceeds the buffer, just return
    else if ((size_t) Serial.availableForWrite() < strlen(msg)){
        err = SERIAL_ERR::ERR_BUF_EXCEEDED;
    }
    else{
        // Send the message and read the response. Anything besides +OK is treated as failure
        sprintf(rylr988Message, "%s%s=%d,%d,%s", MSG_PREFIX, MSG_SEND, destination, strlen(msg), msg);
        serialDevice->println(String(rylr988Message));

        // Read the response and make sure that it was tranmitted without error
        String rsp = serialDevice->readString();
        rsp.trim();
        if (rsp != MSG_OK) {
            err = ERR_TX;
        }
    }

    return err;
}

/// @brief Wait for a message with a given timeout
/// @param msg Buffer to receive message
/// @param bufferSize Length of buffer
/// @param timeout Time to wait
/// @return Result of wait operation, ERR_NO_ERR indicates successful read.
RYLR988::SERIAL_ERR RYLR988::waitMsg(char * msg, u16_t bufferSize, int timeout)
{
    SERIAL_ERR err = ERR_NO_ERROR;
    //int bytesRead = 0;

    unsigned int available = serialDevice->available();
    if (bufferSize < available) {
        return ERR_BUF_EXCEEDED;
    }

    // If there is something to read
    if (available > 0) {
        String response = serialDevice->readString();
        Serial.println(response);

        // Read an incomplete message.  TODO: this should be more robust if there is room left in the buffer.
        if (response.length() < available) {
            err = ERR_INCOMPLETE;
        }
        //bytesRead = Serial.readBytesUntil('\n', msg, bufferSize);
        //msg[bytesRead] = '\0';
    }
    // Otherwise it is an error
    else {
        err = ERR_NO_RX;
    }

    return err;
}


/// @brief Wait for the response from the server to begin operation. Used only in startup.
/// @param timeout time to allow for response in milliseconds
/// @return ERR_TIMEOUT, ERR_NOERROR
RYLR988::SERIAL_ERR RYLR988::waitForServerOK(u16_t timeout)
{
    SERIAL_ERR err = ERR_TIMEOUT;
    char buffer[80];
    String rsp;
    char cmd[12];
    uint16_t length;
    int16_t rssi;
    uint16_t snr;
    uint16_t from;
    char message[80];

    unsigned long expired = millis() + timeout;

    // Wait for the response
    while (Serial.available() == 0 && millis() < expired) {
        delay(250);
        // There is a response from the server
        if (serialDevice->available()) {
            rsp = serialDevice->readString();
            rsp.trim();
            rsp.toCharArray(buffer, 80);


            //The message expected will be of the form: +RCV=[FROM],[LENGTH],[MESSAGE],[RSSI],[SNR]
            int parsed = sscanf(buffer, "%5s%d,%d,%[^,],%d,%d", cmd, &from, &length, message, &rssi, &snr);
            
            sendDebugMessage(message);

            // If the server indicates it is OK to proceed, do so
            if (strcmp(message, MSG_PROCEED) == 0) {
                err = ERR_NO_ERROR;
            }
            else {
                err = ERR_INVALID;
            }

        }
    }
    // Otherwise, just assume that the server did not respond

    return err;
}

/// @brief Translate the error code into something human-readable
/// @param err 
/// @return String of the error code
String RYLR988::errorString(SERIAL_ERR err)
{
    return errorStrings[err];
}

void RYLR988::debug()
{

    serialDevice->println("AT");
    String rsp = serialDevice->readString();
    //Serial.println(rsp);
    serialDevice->println("AT+ADDRESS=20");
    //Serial.println("AT+ADDRESS=20");
    rsp = serialDevice->readString();
    //Serial.println(rsp);
    serialDevice->println("AT+SEND=1,5,hello");
    //Serial.println("AT+SEND=0,5,hello");
    rsp = serialDevice->readString();
    //Serial.println(rsp);

}

/// @brief Set the destination ID for the radio
/// @param id
void RYLR988::setDestination(int id)
{
    destination = id;
}

/// @brief Waits for the OK from the radio
/// @param timeout time in milliseconds to allow for a response 
/// @return true it +OK received, false otherwise
bool RYLR988::commandOK(unsigned long timeout)
{
    bool ok = true;
    char buffer[32];
    unsigned long expired = millis() + timeout;

    // Wait for the radio to send the response
    while (millis() < expired && serialDevice->available() == 0) {
        delay(100);
    }

    // If the radio sent a response, see it it is +OK
    if (serialDevice->available()) {
        String rsp = serialDevice->readString();
        rsp.trim();

        ok = (rsp == MSG_OK);
        // rsp.toCharArray(buffer, 32);
        // sendDebugMessage(buffer);
    }
    else {
        ok = false;
    }

    return ok;
}

/// @brief Extract the actual message, stripping off the radio inserted parts. NOT IMPLEMENTED
/// @param msg
/// @return
String RYLR988::extractActualMsg(char *msg)
{
    return String();
}

#define PREFIX_DEBUG "DEBUG:"
void RYLR988::sendDebugMessage(const char *msg)
{
    char rylrMessage[80];
    char actualMessage[80];

    sprintf(actualMessage, "%s[%s]", PREFIX_DEBUG, msg);
    sprintf(rylrMessage, "%s%s=%d,%d,%s", MSG_PREFIX, MSG_SEND, destination, strlen(actualMessage), actualMessage);
    serialDevice->println(String(rylrMessage));
    serialDevice->readString();
}

/// @brief Get the UID of the radio
/// @return 12 byte identifier
char *RYLR988::getUID()
{
    char buffer[80];
    char cmd[12];
    int len = 0;

    ping();
    // If the UID has not already been retrieved, do so
    if (strlen(uid) == 0) {
        sprintf(buffer, "%s%s%s", MSG_PREFIX, MSG_UID, QUERY);
        serialDevice->println(String(buffer));

        if (serialDevice->available() > 0) {
            char debug[80];
            // sprintf(debug, "Bytes to read: %d", serialDevice->available());
            // sendDebugMessage(debug);
            String rsp = serialDevice->readString();
            rsp.trim();
            rsp.toCharArray(debug, 80);
            sendDebugMessage(debug);
            Serial.println(debug);
        }
        else {
            sendDebugMessage("OK");
        }
        serialDevice->println(String(buffer));

        String rsp=serialDevice->readString();
        rsp.toCharArray(buffer, 80);
        Serial.println(buffer);

        //The message expected will be of the form: +UID=OF...0
        if(sscanf(buffer, "%5s%s", cmd, uid) != 2) {
            strcpy(uid,"UNKNOWN");
        }
        // Debug
        //sendDebugMessage(value);
    }

    return uid;
}

uint16_t RYLR988::getAddress()
{
    return address;
}
