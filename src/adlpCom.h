#pragma once
/*
 *  adlpCom.h
 *  abbrobot
 *
 *  Created by Jonas Jongejan on 18/07/11.
 *
 */

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


class ADLPCom : public ofxThread{
public:
	ADLPCom();
	
	void update();

	void queueMessage(ARAPMessage message);
    vector<ARAPMessage> readMessages(); 
    vector<ARAPMessage> readMessagesAfterCount(long count);

    long readMessageCounter; //For each new message that is received this is ++
    
	bool connected;
	bool receivingMessage;
    bool sendingMessage;
	
private:
	ofSerialEven serial;
    
    int incommingMessageIndex;
    ARAPMessage incommingMessageTemp;    
    bool incommingFormalities[5];
    bool waitingForResponse;
    
    void parseIncommingByte(unsigned char bytesReturned);
    
    vector<ARAPMessage> messageQueue;
    vector<ARAPMessage> incommingMessages;
    
    bool outgoingFormalities[2];
    
    void threadedFunction();
    
    int multimessageDataIndex;
    bool sendingMultimessage;
    bool readyToSendNextMultiMessage;
    
};