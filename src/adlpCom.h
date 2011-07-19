#pragma once
/*
 *  adlpCom.h
 *  abbrobot
 *
 *  Created by Jonas Jongejan on 18/07/11.
 *
 */

#include "ofSerialEven.h"


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

enum ADLPMessageType {
	query = 0x01,
	response = 0x02,
	automatic = 0x03,
	warning = 0x06,
	multifrompc = 0x09,
	multifromcontroller = 0x0A
};

enum ADLPInstruction {
    uploadProgram = 0x01,
    startProgram = 0x02,
    stopProgram = 0x03, //confirmed
    TCPvalue = 0x04,
    locationValue = 0x05,
    registerValue = 0x06,
    sensorValue = 0x07,
    configurationValue = 0x0A,
    frameValue = 0x0B,
    statusList = 0x13,
    operationMode = 0x14,
    programList = 0x15,
    eraseProgram = 0x16,
    arcWeldValue = 0x19,
    downloadProgram = 0x1D,
	ARAPVersion = 0x2D,
    resolverValue = 0x2E,
    automaticStatusUpdate = 0x7F //return from robot at emergency break
};

struct ADLPMessage {
	unsigned char size;
	bool fromComputer;
	ADLPInstruction instruction;
	ADLPMessageType messageType;
	unsigned char * data;	
};


class ADLPCom{
public:
	ADLPCom();
	
	void update();
	void sendMessage();
    void sendMessage(ADLPInstruction instruction, unsigned char * data, int datasize);
	void sendMessage(ADLPMessage message);
    
	bool connected;
	bool receivingMessage;
    bool sendingMessage;
	
private:
	ofSerialEven serial;
    
    int incommingMessageIndex;
    ADLPMessage incommingMessageTemp;    
    bool incommingFormalities[5];
    
    void parseIncommingByte(unsigned char bytesReturned);
    
    vector<ADLPMessage> messageQueue;
    bool outgoingFormalities[2];
};