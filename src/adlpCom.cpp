/*
 *  adlpCom.cpp
 *  abbrobot
 *
 *  Created by Jonas Jongejan on 18/07/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "adlpCom.h"

ADLPCom::ADLPCom(){
	//Start serial communication
	connected = serial.setup(0,9600);
	
	serial.enumerateDevices();
	
	
}

void ADLPCom::update(){
	unsigned char bytesReturned;
    while(serial.available()  > 0){
        (bytesReturned = serial.readByte());
		switch (bytesReturned) {
			case ACK:
				printf("Recv: %x (ACK)",bytesReturned);
				break;
			case ENQ:
				printf("Recv: %x (ENQ)",bytesReturned);
				break;
			case NAK:
				printf("Recv: %x (NAK)",bytesReturned);
				break;
			default:
				printf("Recv: %x",bytesReturned);
				break;
		}
        cout<<bytesReturned<<endl;
        if(bytesReturned == 0x05){
            serial.writeByte(ACK);
        }
    };
	
}

void ADLPCom::sendMessage(){
	unsigned char message[6];
	memset(message, 0, 6);
	
	message[1] = 6; //Size of text message
	message[2] = 1; //Mesesage from PC to controller
	message[3] = 0; //Mesesage from PC to controller
	message[4] = 0x2D; // Instruction
	message[5] = 1;
	//		message[7] = 1;
	
	
	serial.writeByte(ENQ);
	serial.writeByte(DLE);
	serial.writeByte(STXodd);
	serial.writeBytes(message, 6);
	serial.writeByte(DLE);
	serial.writeByte(ETX);
	serial.writeByte(message[4] ^ ETX);
}