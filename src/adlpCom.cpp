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
	
    incommingMessageIndex = -1;
    receivingMessage = false;
    sendingMessage = false;
    memset(outgoingFormalities, false, 2);
    
    startThread(true, false);
	
}

void ADLPCom::update(){
	
}


void ADLPCom::queueMessage(ARAPMessage msg){
    if(lock()){
        messageQueue.push_back(msg);
        unlock();
    } 
}

bool ADLPCom::available(){
    bool ret = false;
    if(lock()){
        ret = (incommingMessages.size() > 0);
        unlock();
    }
    return ret; 
}

ARAPMessage ADLPCom::readMessage(){
    ARAPMessage ret;
    if(lock()){
        ret = incommingMessages[0];
        incommingMessages.erase(incommingMessages.begin());
        unlock();
    }
    return ret;
}

void ADLPCom::threadedFunction(){
    while( isThreadRunning() != 0 ){
        if( lock() ){
            unsigned char bytesReturned;
            while(serial.available()  > 0){
                bytesReturned = serial.readByte();
                
                printf("%i: ",incommingMessageIndex);
                switch (bytesReturned) {
                    case ACK:
                        printf("Recv: %x (ACK)\n",bytesReturned);
                        break;
                    case DLE:
                        printf("Recv: %x (DLE)\n",bytesReturned);
                        break;
                    case EOT:
                        printf("Recv: %x (EOT)\n",bytesReturned);
                        break;
                    case ETX:
                        printf("Recv: %x (ETX)\n",bytesReturned);
                        break;
                    case ENQ:
                        printf("Recv: %x (ENQ)\n",bytesReturned);
                        break;
                    case NAK:
                        printf("Recv: %x (NAK)\n",bytesReturned);
                        break;
                    default:
                        printf("Recv: %x\n",bytesReturned);
                        break;
                } 
                
                //This is where the parsing is happening
                parseIncommingByte(bytesReturned);
                
            }
            
            if(messageQueue.size() > 0 && !receivingMessage){
                //There are messages to be send
                if(!sendingMessage){
                    //ENQ not send yet, lets send it
                    serial.writeByte(ENQ);
                    sendingMessage = true;
                    outgoingFormalities[0] = true;
                } else if(outgoingFormalities[1]){
                    ARAPMessage msg = messageQueue[0];
                    messageQueue.erase(messageQueue.begin());
                    
                    printf("Send message %x\n",msg.instruction);
                    
                    unsigned char message[msg.size];
                    memset(message, 0, msg.size);
                    
                    message[1] = msg.size; //Size of text message
                    message[2] = 1; //Mesesage from PC to controller
                    message[3] = 0; //Mesesage from PC to controller
                    message[4] = msg.instruction; // Instruction
                    message[5] = msg.messageType;
                    message[6] = 0; //function suffix
                    message[7] = msg.functionSuffix; //function suffix
                    for(int i=0;i<msg.size-8;i++){
                        message[8+i] = msg.data[i];
                    }
                    
                    unsigned char checksum = message[0];
                    for(int i=1;i<msg.size;i++){
                        checksum = checksum ^ message[i];
                    }
                    //		message[7] = 1;
                    
                    
                    serial.writeByte(DLE);
                    serial.writeByte((msg.size % 2)? STXodd : STXeven);
                    serial.writeBytes(message, msg.size);
                    serial.writeByte(DLE);
                    serial.writeByte(ETX);
                    serial.writeByte(checksum ^ ETX);
                }
            }
            unlock();
            usleep(1000*100);
        }
    }
}


void ADLPCom::parseIncommingByte(unsigned char bytesReturned){
    if(bytesReturned == ACK && sendingMessage){
        if(outgoingFormalities[0] && outgoingFormalities[1]){
            //Sending was succesfull
            outgoingFormalities[0] = false;
            outgoingFormalities[1] = false;
            sendingMessage = false;
        } else if(outgoingFormalities[0] && !outgoingFormalities[1])  {
            //Start sending message
            outgoingFormalities[1] = true;
        }
    }
    if(bytesReturned == ENQ){
        //New message is incomming
        //Return with an acknowledge 
        serial.writeByte(ACK);
        receivingMessage = true;
        incommingMessageIndex = 0; //Reset the index counter
        memset(incommingFormalities,false,5); //reset the formalities handler
        incommingFormalities[0] = true; //Tell that the first formality is met
        
    } else if(incommingMessageIndex >= 0 && incommingFormalities[0] && incommingFormalities[1] && incommingFormalities[2] && !incommingFormalities[3] && !incommingFormalities[4] && (incommingMessageIndex <= 1 || incommingMessageIndex < incommingMessageTemp.size) ){
        //We are processing a message content that is incomming
        switch (incommingMessageIndex) {
            case 0:
                //Not used
                break;
            case 1:
                cout<<"Start new message with size "<<(int)bytesReturned<<endl;
                incommingMessageTemp.size = bytesReturned;
                incommingMessageTemp.data = (unsigned char*) malloc(incommingMessageTemp.size*sizeof(unsigned char));
                break;
            case 2:
                if(bytesReturned == 1){
                    incommingMessageTemp.fromComputer = true; //How should this be possible?! 
                } else {
                    incommingMessageTemp.fromComputer = false;
                }
                break;                    
            case 3:
                break;   
            case 4:
                incommingMessageTemp.instruction = (ARAPInstruction)bytesReturned;
                break;
            case 5:
                incommingMessageTemp.messageType = (ARAPMessageType)bytesReturned;
                if(incommingMessageTemp.messageType  == warning){
                    cout<<"WARNING"<<endl;
                }
                break;
            case 6:
                //function
                break;
            case 7:
                //function
                break;
                
            default:
                //Everything else is data
                printf("\t\t\t\tData: %x %i\n",bytesReturned,bytesReturned);
                if(bytesReturned > 31 && bytesReturned < 127)
                    printf("%c",bytesReturned);
                incommingMessageTemp.data[incommingMessageIndex-8] = bytesReturned;
                break;
        }
        
        incommingMessageIndex++;
    } else if(bytesReturned == DLE){
        if(!incommingFormalities[1]){
            incommingFormalities[1] = true;            
            cout<<"First DLE"<<endl;
        }else {
            incommingFormalities[3] = true; //Second time DLE is received after message         
            cout<<"Second DLE"<<endl;
        }
    } else if(bytesReturned == STXodd || bytesReturned == STXeven){
        incommingFormalities[2] = true;       
        cout<<"Got stx"<<endl;
    } else if(bytesReturned == ETX){
        //End of text
        cout<<"got etx"<<endl;
        incommingFormalities[4] = true;
    } else if(bytesReturned == EOT){
        //End of transmission
        cout<<"got EOT"<<endl;
        printf("Done receiving at msg %i\n",incommingMessageIndex);
        printf("Received instruction: %i",incommingMessageTemp.instruction );
        if(incommingMessageTemp.messageType == warning){
            cout<<"WARNING"<<endl;
        }
        cout<<incommingMessageTemp.data<<endl;
        incommingMessageIndex = -1;
        incommingMessages.push_back(incommingMessageTemp);
        incommingMessageTemp = ARAPMessage();
    } 
    else if(incommingMessageIndex >= 0 && incommingFormalities[0] && incommingFormalities[1] && incommingFormalities[2] && incommingFormalities[3] && incommingFormalities[4]){
        printf("Recevied checksum %x\n",bytesReturned);
        //All formalities met, check cheksum (TODO)
        serial.writeByte(ACK); 
        receivingMessage = false;
        
    }        
}
