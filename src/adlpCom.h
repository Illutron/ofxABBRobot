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



class ADLPCom : public ofxThread{
public:
	ADLPCom();
	
	void update();

	void queueMessage(ARAPMessage message);
    bool available(); //if there is a readMessage
    ARAPMessage readMessage(); 
    void markRead(); //Mark the incomming message as read (so nobody else reads it)
    
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
    ARAPMessage incommingMessage;
    bool messageRead;
    
    bool outgoingFormalities[2];
    
    void threadedFunction();
    
};