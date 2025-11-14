//
// R L R 9 9 8 . C P P
//
// RLRY998 Support
//

#include "RYLR988.h"
#include "Arduino.h"

// Messages that can be sent to the radio

#define MSG_OK          "+OK"
#define MSG_MODE        "+MODE"
#define MSG_IPR         "+IPR"
#define MSG_BAND        "+BAND"
#define MSG_PARAMETER   "+PARAMETER"
#define MSG_ADDRESS     "+ADDRESS"
#define MSG_NETWORK     "+NETWORK"
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

// Pre-defined messages
#define MSG_PROCEED     "OK"
#define MSG_DUPLICATE   "DUPLICATE"

#define SELECT_ADDRESS  0
#define MIN_ADDRESS     2
#define MAX_ADDRESS     1024

/// @brief RYLR998 Support
RYLR988::RYLR988()
{
    // The default radio destination
    destination = 1;
}

/// @brief Set up the radio
/// @param baud The baud -- 115200 is commonly used for the RYLT998
/// @param address  The address of this radio -- 0 indicates an address will be chosen
/// @param network The network
/// @return ERR_NO_ERR on success
RYLR988::SERIAL_ERR RYLR988::begin(uint32_t baud, u16 address, u16 network)
{
    char configurationMsg[80];
    char helloMessage[80];
    SERIAL_ERR err;
    String rsp;

    char *radioID;

    Serial.begin(baud);

    // Check to see if the chip is ready to accept commands
    err = ping();

    if (err == ERR_NO_ERROR) {
        if (address == SELECT_ADDRESS) {
                randomSeed(analogRead(0));
                address = random(MIN_ADDRESS, MAX_ADDRESS);
        }
        // Configure the address
        sprintf(configurationMsg, "%s%s=%d", MSG_PREFIX, MSG_ADDRESS, address);
        Serial.println(String(configurationMsg));
        rsp = Serial.readString();

        // Configure the network
        sprintf(configurationMsg, "%s%s=%d", MSG_PREFIX, MSG_NETWORK, network);
        Serial.println(String(configurationMsg));
        rsp = Serial.readString();

        getUID();
        radioID = uid;
        // Announce this system
        sprintf(helloMessage, "begin:%s", radioID);
        sprintf(configurationMsg, "%s%s=%d,%d,%s", MSG_PREFIX, MSG_SEND,  destination, strlen(helloMessage), helloMessage);
        Serial.println(String(configurationMsg));

        // This is just the acknowledgement
        rsp = Serial.readString();

        // The server will send an ack for the startup.
        err = waitForOK(4000);
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

    // The device responds with +OK when presented with AT
    Serial.println(MSG_PREFIX);


    if (err == ERR_NO_ERROR) {
        String rx = Serial.readString();
        rx.trim();
        if (rx != MSG_OK) {
            err = ERR_RADIO_NOT_READY;
        }

    }

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
    if (Serial.available() > 0){
        err = SERIAL_ERR::ERR_READ_REQ;
    }
    // If the message exceeds the buffer, just return
    else if ((size_t) Serial.availableForWrite() < strlen(msg)){
        err = SERIAL_ERR::ERR_BUF_EXCEEDED;
    }
    else{
        // Send the message and read the response. Anything besides +OK is treated as failure
        sprintf(rylr988Message, "%s%s=%d,%d,%s", MSG_PREFIX, MSG_SEND, destination, strlen(msg), msg);
        Serial.println(String(rylr988Message));

        // Read the response and make sure that it was tranmitted without error
        String rsp = Serial.readString();
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
RYLR988::SERIAL_ERR RYLR988::waitMsg(char * msg, u16 bufferSize, int timeout)
{
    SERIAL_ERR err = ERR_NO_ERROR;
    //int bytesRead = 0;

    unsigned int available = Serial.available();
    if (bufferSize < available) {
        return ERR_BUF_EXCEEDED;
    }

    // If there is something to read
    if (available > 0) {
        String response = Serial.readString();
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
RYLR988::SERIAL_ERR RYLR988::waitForOK(u16 timeout)
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
        if (Serial.available()) {
            rsp = Serial.readString();
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

    Serial.println("AT");
    String rsp = Serial.readString();
    //Serial.println(rsp);
    Serial.println("AT+ADDRESS=20");
    //Serial.println("AT+ADDRESS=20");
    rsp = Serial.readString();
    //Serial.println(rsp);
    Serial.println("AT+SEND=1,5,hello");
    //Serial.println("AT+SEND=0,5,hello");
    rsp = Serial.readString();
    //Serial.println(rsp);

}

/// @brief Set the destination ID for the radio
/// @param id
void RYLR988::setDestination(int id)
{
    destination = id;
}

bool RYLR988::commandOK()
{
    bool ok = true;
    char buffer[32];

    String rsp = Serial.readString();
    rsp.trim();

    sprintf(buffer, "RX: %s", rsp.c_str());
    sendDebugMessage(buffer);
    ok = (rsp == MSG_OK);
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
    Serial.println(String(rylrMessage));
    Serial.readString();
}

/// @brief Get the UID of the radio
/// @return 12 byte identifier
char *RYLR988::getUID()
{
    char buffer[80];
    char *token;
    char *value;
    int len = 0;

    // If the UID has not already been retrieved, do so
    if (strlen(uid) == 0) {
        sprintf(buffer, "%s%s%s", MSG_PREFIX, MSG_UID, QUERY);
        Serial.println(String(buffer));

        String rsp=Serial.readString();
        rsp.toCharArray(buffer, 80);

        //The message expected will be of the form: +UID=OF...0
        token = strtok(buffer, SEPARATOR);
        value = strtok(NULL, SEPARATOR);
        
        // // Replace the newline
        value[strcspn(value, "\n")] = 0;
        value[strcspn(value, "\r")] = 0;
        value[strlen(value) - 1] = 0;

        strncpy(uid, value, strlen(value));

        // Debug
        //sendMsg(value);
    }


    return uid;
}

uint16_t RYLR988::getAddress()
{
    return address;
}
