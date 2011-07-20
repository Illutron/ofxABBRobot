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
    waitingForResponse = false;
    memset(outgoingFormalities, false, 2);
    readMessageCounter = 0;
    readyToSendNextMultiMessage = false;
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


vector<ARAPMessage> ADLPCom::readMessages(){
    vector<ARAPMessage> ret;
    if(lock()){
        ret = incommingMessages;
        unlock();
    }
    return ret;
}

vector<ARAPMessage> ADLPCom::readMessagesAfterCount(long count){
    vector<ARAPMessage> ret ;   
    long diff = readMessageCounter - count;
    if(diff <= 0){
        return ret;
    } else {
        vector<ARAPMessage> v = readMessages();
        for(int i=0;i<diff;i++){
            ret.push_back(v[v.size()-1-i]);
        }        
        return ret;        
    }
}

void ADLPCom::threadedFunction(){
    while( isThreadRunning() != 0 ){
        if( lock() ){
            unsigned char bytesReturned;
            while(serial.available()  > 0){
                bytesReturned = serial.readByte();
                
                printf("%i/%i: ",incommingMessageIndex, incommingMessageTemp.size);
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
            
            if(messageQueue.size() > 0 && !receivingMessage && !waitingForResponse){
                //There are messages to be send
                if(!sendingMessage){
                    //ENQ not send yet, lets send it
                    serial.writeByte(ENQ);
                    sendingMessage = true;
                    outgoingFormalities[0] = true;
                } else if(outgoingFormalities[1] || readyToSendNextMultiMessage){
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
                    printf("Message type: %x\n",msg.messageType);
                    message[6] = 0; //function suffix << 8 TODO
                    message[7] = msg.functionSuffix; //function suffix
                    for(int i=0;i<msg.size;i++){
                        message[8+i] = msg.data[i];
                        cout<<"Add data: "<<8+i<<"  "<<(int)msg.data[i]<<endl;
                    }

                    readyToSendNextMultiMessage = false;
                    if(msg.messageType == multifrompc){
                        sendingMultimessage = true;
                    } else {
                        sendingMultimessage = false;
                    }
                    
                    
                    
                    unsigned char checksum = message[0];
                    for(int i=1;i<msg.size+8;i++){
                        checksum = checksum ^ message[i];
                    }
                    //		message[7] = 1;
                    
                        
                    serial.writeByte(DLE);
                    serial.writeByte((msg.size % 2)? STXodd : STXeven);
                    serial.writeBytes(message, msg.size+8);
                    serial.writeByte(DLE);
                    serial.writeByte(ETX);
                    serial.writeByte(checksum ^ ETX);
                    if(msg.messageType == query){
                        waitingForResponse = true;
                    }
                }
            }
            unlock();
            usleep(1000*100);
        }
    }
}


void ADLPCom::parseIncommingByte(unsigned char bytesReturned){
    if(bytesReturned == ACK && sendingMessage){
        cout<<"Got ack, and is sending message "<<outgoingFormalities[0]<<"  "<< outgoingFormalities[1]<<endl;
        if(outgoingFormalities[0] && outgoingFormalities[1]){
                    cout<<"Formalities ok"<<endl;
            //Sending was succesfull
            outgoingFormalities[0] = false;
            outgoingFormalities[1] = false;
            if(!sendingMultimessage){
                sendingMessage = false;
                cout<<" Send EOT "<<endl;
                serial.writeByte(EOT);
            } else {
                outgoingFormalities[0] = true;
                outgoingFormalities[1] = true;
                readyToSendNextMultiMessage = true;
            }
        } else if(outgoingFormalities[0] && !outgoingFormalities[1])  {
            //Start sending message
            outgoingFormalities[1] = true;
        }
    }
    if(incommingMessageIndex >= 0 && incommingFormalities[0] && incommingFormalities[1] && incommingFormalities[2] && !incommingFormalities[3] && !incommingFormalities[4] && (incommingMessageIndex <= 1 || incommingMessageIndex + multimessageDataIndex < incommingMessageTemp.size+8) ){
        //We are processing a message content that is incomming
        switch (incommingMessageIndex) {
            case 0:
                //Not used
                break;
            case 1:
            {
                cout<<"Start new message with size "<<(int)bytesReturned<<endl;
                int oldSize = incommingMessageTemp.size;
                unsigned char  temp[oldSize];
                if(oldSize > 0){
                    cout<<"Already "<<(int)oldSize<<" bytes in the message to add to"<<endl;                    
                    for(int i=0;i<oldSize;i++){
                        temp[i] = incommingMessageTemp.data[i];
                    }
                    //                    memcpy(temp , incommingMessageTemp.data, oldSize*sizeof(unsigned char));                    
                    //Now we got a copy of the old data, so we can allocate a bigger one
                    free(incommingMessageTemp.data);
                }
                incommingMessageTemp.size += bytesReturned-8;
                incommingMessageTemp.data = (unsigned char*) malloc((incommingMessageTemp.size+100)*sizeof(unsigned char));
                
                if(oldSize > 0){
                    //If we had some old data, lets copy it back
                    for(int i=0;i<oldSize;i++){
                        incommingMessageTemp.data[i] = temp[i];
                    }
                    //                    memcpy(incommingMessageTemp.data, temp, oldSize*sizeof(unsigned char));
                    multimessageDataIndex = oldSize;
                    //  free(temp);
                }
            }
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
                break;
            case 6:
                incommingMessageTemp.functionSuffix = bytesReturned<<8;
                break;
            case 7:
                incommingMessageTemp.functionSuffix += bytesReturned;
                break;
                
            default:
                //Everything else is data
                printf("\t\t\t\tData %i: %x %i\n",multimessageDataIndex+incommingMessageIndex-8, bytesReturned,bytesReturned);
                if(bytesReturned > 31 && bytesReturned < 127)
                    printf("%c",bytesReturned);
                incommingMessageTemp.data[multimessageDataIndex+incommingMessageIndex-8] = bytesReturned;
                break;
        }
        
        incommingMessageIndex++;
    }  else  if(bytesReturned == ENQ){
        //New message is incomming
        //Return with an acknowledge 
        serial.writeByte(ACK);
        receivingMessage = true;
        incommingMessageIndex = 0; //Reset the index counter
        memset(incommingFormalities,false,5); //reset the formalities handler
        incommingFormalities[0] = true; //Tell that the first formality is met
        multimessageDataIndex = 0;
        incommingMessageTemp.size = 0;
        
    }  else if(bytesReturned == DLE){
        if(!incommingFormalities[1]){
            incommingFormalities[1] = true;            
            cout<<"First DLE"<<endl;
        }else if(!incommingFormalities[3]) {
            incommingFormalities[3] = true; //Second time DLE is received after message         
            cout<<"Second DLE"<<endl;
        } else {
            //New message
            cout<<"----------"<<endl<<"Next message"<<endl;
            incommingMessageIndex = 0; //Reset the index counter
            memset(incommingFormalities,false,5); //reset the formalities handler
            incommingFormalities[0] = true;
            incommingFormalities[1] = true; 
            receivingMessage = true;
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
        //if(incommingMessageTemp.instruction != 0x7F){
        waitingForResponse = false;
        printf("Done receiving at msg %i\n",incommingMessageIndex);
        printf("Received instruction: %i",incommingMessageTemp.instruction );
        incommingMessageIndex = -1;
        incommingMessages.push_back(incommingMessageTemp);
        readMessageCounter ++;
        while (incommingMessages.size() > incommingBufferSize){
            incommingMessages.erase(incommingMessages.begin());
        }
        incommingMessageTemp = ARAPMessage();
        //  }
    } 
    else if(incommingMessageIndex >= 0 && incommingFormalities[0] && incommingFormalities[1] && incommingFormalities[2] && incommingFormalities[3] && incommingFormalities[4]){
        printf("Recevied checksum %x\n",bytesReturned);
        //All formalities met, check cheksum (TODO)
        serial.writeByte(ACK); 
        receivingMessage = false;
        
        /* if(incommingMessageTemp.instruction == 0x7F){
         //HACK: When we receive automatic status updates, we don't always get a EOT, so we cheat here. I don't know why
         waitingForResponse = false;
         printf("Received instruction: %i",incommingMessageTemp.instruction );
         incommingMessageIndex = -1;
         incommingMessage = incommingMessageTemp;
         messageRead = false;
         incommingMessageTemp = ARAPMessage();
         
         }*/
    }        
}
