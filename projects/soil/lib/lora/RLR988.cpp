#include "RYLR988.h"
#include "Arduino.h"

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
#define MSG_FCCT "AT+FCCT"

#define MSG_PREFIX "AT"

#define SEPARATOR "="

RYLR988::RYLR988()
{
    // The default radio 
    destination = 1;
}

RYLR988::SERIAL_ERR RYLR988::begin(uint32_t baud, u16 address, u16 network)
{
    char configurationMsg[80];
    SERIAL_ERR err;

    Serial.begin(baud);

    // Check to see if the chip is ready to accept commands
    err = ping();

    Serial.println("AT");
    String rsp = Serial.readString();
    //Serial.println(rsp);
    sprintf(configurationMsg, "AT+ADDRESS=%d", address);
    Serial.println(String(configurationMsg));
    //Serial.println("AT+ADDRESS=20");
    rsp = Serial.readString();
    //Serial.println(rsp);
    Serial.println("AT+SEND=1,5,hello");
    //Serial.println("AT+SEND=0,5,hello");
    rsp = Serial.readString();
    //Serial.println(rsp);

    // if (err == ERR_NO_ERROR) {
    //     // Configure Address
    //     sprintf(configurationMsg, "%s+%s=%d", MSG_PREFIX, MSG_ADDRESS, address);
    //     sendMessage(configurationMsg);

    //     // Configure Network
    //     sprintf(configurationMsg, "%s+%s=%d", MSG_PREFIX, MSG_NETWORK, network);
    //     sendMessage(configurationMsg);
    // }

    return (err);
    
}

LoraConfiguration RYLR988::getLoraConfiguration()
{
    return LoraConfiguration();
}

int RYLR988::setLoraConfiguration(LoraConfiguration cfg)
{
    return 0;
}

/// @brief Ping the radio to make sure it is ready for commands
/// @return ERR_NO_ERROR if it is OK, ERR_NO_RADIO or ERR_RADIO_NOT_READY if not

RYLR988::SERIAL_ERR RYLR988::ping()
{
    SERIAL_ERR err = ERR_NO_ERROR;
    char rxMessage[80];
    char msg[] = MSG_PREFIX;

    //err = sendMessage(msg);
    Serial.println(MSG_PREFIX);
    String rsp = Serial.readString();
    Serial.println(rsp);
    return ERR_NO_ERROR;

    if (err == ERR_NO_ERROR) {
        String rx = Serial.readString();
        rx.toCharArray(lastMessage, 80);
        if (rx != MSG_OK) {
            err = ERR_RADIO_NOT_READY;
        }
        // Serial.println(rx);
        //err = waitMsg(rxMessage, 80, 0);
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
        sprintf(rylr988Message, "AT+SEND=%d,%d,%s", destination, strlen(msg), msg);
        Serial.println(String(rylr988Message));
        String rsp = Serial.readString();
        rsp.trim();
        if (rsp != MSG_OK) {
            err = ERR_TX;
        }
    }

    //Serial.println(rsp);
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

void RYLR988::setDestination(int i)
{
    destination = i;
}

String RYLR988::extractActualMsg(char *msg)
{
    return String();
}

RYLR988::SERIAL_ERR RYLR988::sendMessage(char *msg)
{
    return ERR_NO_ERROR;
}
