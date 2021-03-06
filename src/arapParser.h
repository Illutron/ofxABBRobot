#pragma once
//#include "adlpCom.h"
#include "arapInstructions.h"

enum ARAPInstruction {
    SENDPROGRAM = 1,
    STARTPROGRAM = 2,
    STOPPROGRAM = 3, //confirmed
    READPROGRAMSTATUS = 21,
    DELETEPROGRAM = 22,
    DISKLOADPROGRAM = 23,
    RECEIVEPROGRAM = 29,
    READTCP = 4,
    READLOCATION = 5,
    READREGISTER = 6,
    READSENSOR = 7,
    READINPUT = 8,
    READOUTPUT = 9,
    READCONFIG = 10,
    READFRAME = 11,
	READSTATUS = 19,
    WRITETCP = 12,
    WRITELOCATION = 13,
    WRITEREGISTER = 14,
    WRITESENSOR = 15,
    WRITEOUTPUT = 16,
    WRITECONFIG = 17,
	WRITEFRAME = 18,
    WRITEMODE = 20,
    MOVE = 24    
};

enum ARAPMessageType {
	query = 0x01,
	response = 0x02,
	automatic = 0x03,
	warning = 0x06,
	multifrompc = 0x09,
	multifromcontroller = 0x0A
};


struct ARAPMessage {
	unsigned char size;
	bool fromComputer;
	ARAPInstruction instruction;
	ARAPMessageType messageType;
    int functionSuffix;
	unsigned char * data;	
    int ort;
};


class ARAPParser {
public:
    ARAPMessage constructMessage(ARAPInstruction instruction, int functionSuffix = 0, unsigned char * data = nil, int datasize = 0);
    
    ARAPMessage constructMoveMessage(ARAP_COORDINATE coord, int velocity, int runSpeed, int functionSuffix, unsigned char moveData);
    
    ARAP_STATUS parseStatusMessage(ARAPMessage msg);
    
    ARAP_COORDINATE coordinateFromRaw(ARAP_COORDINATE_RAW raw);
    ARAP_COORDINATE_RAW coordinateToRaw(ARAP_COORDINATE coord);    
};