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
	ARAPVersion = 0x2D
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
	
	bool connected;
	
	
private:
	ofSerialEven serial;
};