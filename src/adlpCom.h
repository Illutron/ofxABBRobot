#pragma once


#include "ofSerialEven.h"
#include "arapParser.h"
#include "ofxThread.h"

#define ENQ 0x05
#define ACK 0x06
#define WACK 0x0E
#define RVI 0x0F
#define NAK 0x15
#define DLE 0x10
#define STXeven 0x02
#define STXodd 0x82
#define ETX 0x03
#define EOT 0x04

#define incommingBufferSize 100

#define ADLP_DEBUG 0


class ADLPCom : public ofxThread{
public:
	ADLPCom();

    //add message to the outgoing message queue
	void queueMessage(ARAPMessage message);
    
    //Get all messaages in the inbox
    vector<ARAPMessage> readMessages(); 
    //Get all messages newer then a certain message count in the inbox (usefull for checking for deltas)
    vector<ARAPMessage> readMessagesAfterCount(long count);

    //For each new message that is received this will ++
    long readMessageCounter; 
    
    //Is the robot connected (serial)
	bool connected;
    
    //Status indications
	bool receivingMessage;
    bool sendingMessage;
	
private:
	ofSerialEven serial;
    
    //Determines what to do with each incomming byte from the controller
    void parseIncommingByte(unsigned char bytesReturned);
    
    //Inbox stuff
    vector<ARAPMessage> incommingMessages; //the inbox
    int incommingMessageIndex;
    ARAPMessage incommingMessageTemp;    
    bool incommingFormalities[5];
    bool waitingForResponse;
    
    //Outgoing stuff
    vector<ARAPMessage> messageQueue; //the outbox    
    bool outgoingFormalities[2];    
    int multimessageDataIndex;
    bool sendingMultimessage;
    bool readyToSendNextMultiMessage;
    vector<unsigned char> lastSending;
    
    //The loop for the serial stuff
    void threadedFunction();
    
};