#include "adlpCom.h"

ADLPCom::ADLPCom(){
	//Start serial communication
	connected = serial.setup(0,9600);
    
    if(ADLP_DEBUG){
        serial.enumerateDevices();
    }
	
    //Initialize variables
    incommingMessageIndex = -1;
    receivingMessage = false;
    sendingMessage = false;
    waitingForResponse = false;
    memset(outgoingFormalities, false, 2);
    readMessageCounter = 0;
    readyToSendNextMultiMessage = false;
    
    startThread(true, false); 
}

//----------------------

void ADLPCom::queueMessage(ARAPMessage msg){
    if(lock()){
        messageQueue.push_back(msg);
        unlock();
    } 
}

//----------------------

vector<ARAPMessage> ADLPCom::readMessages(){
    vector<ARAPMessage> ret;
    if(lock()){
        ret = incommingMessages;
        unlock();
    }
    return ret;
}

//----------------------

vector<ARAPMessage> ADLPCom::readMessagesAfterCount(long count){
    vector<ARAPMessage> ret;   
    long diff = readMessageCounter - count;
    if(diff <= 0){
        //no new messages
        return ret; 
    } else {
        vector<ARAPMessage> v = readMessages();
        for(int i=0;i<diff;i++){
            ret.push_back(v[v.size()-1-i]);
        }        
        return ret;        
    }
}

//----------------------

void ADLPCom::threadedFunction(){
    while( isThreadRunning() != 0 ){
        if( lock() ){
            unsigned char bytesReturned;
            while(serial.available()  > 0){
                bytesReturned = serial.readByte();
                
                if(ADLP_DEBUG){
                    printf("%i/%i: ",incommingMessageIndex, incommingMessageTemp.size);
                    switch (bytesReturned) {
                        case ACK:
                            printf("Recv: %i (ACK)\n",bytesReturned);
                            break;
                        case DLE:
                            printf("Recv: %i (DLE)\n",bytesReturned);
                            break;
                        case EOT:
                            printf("Recv: %i (EOT)\n",bytesReturned);
                            break;
                        case ETX:
                            printf("Recv: %i (ETX)\n",bytesReturned);
                            break;
                        case ENQ:
                            printf("Recv: %i (ENQ)\n",bytesReturned);
                            break;
                        case NAK:
                            printf("Recv: %i (NAK)\n",bytesReturned);
                            break;
                        default:
                            printf("Recv: %i (int) %x (hex)\n",bytesReturned,bytesReturned);
                            break;
                    } 
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
                    //We got an ACK in return from the controller,
                    //so we can send the content of the next message
                    ARAPMessage msg = messageQueue[0];
                    messageQueue.erase(messageQueue.begin());
                    
                    if(ADLP_DEBUG){
                        printf("\nSend message %i\n",msg.instruction);
                        printf("Message type: %x\n",msg.messageType);
                    }
                    
                    unsigned char message[msg.size];
                    memset(message, 0, msg.size);
                    
                    message[1] = msg.size; //Size of text message
                    message[2] = 1; //Mesesage from PC to controller
                    message[3] = 0; //Mesesage from PC to controller
                    message[4] = msg.instruction; // Instruction
                    message[5] = msg.messageType; 
                    message[6] = 0; //function suffix << 8 TODO, but never used
                    message[7] = msg.functionSuffix; //function suffix
                    for(int i=0;i<msg.size;i++){
                        message[8+i] = msg.data[i];
                        if(ADLP_DEBUG) cout<<"Add data: "<<8+i<<"  "<<(int)msg.data[i]<<endl;
                    }
                    
                    readyToSendNextMultiMessage = false;
                    if(msg.messageType == multifrompc){
                        sendingMultimessage = true;
                    } else {
                        sendingMultimessage = false;
                    }                    
                    
                    //Calculate checksum
                    unsigned char checksum = message[0];
                    for(int i=1;i<msg.size+8;i++){
                        checksum = checksum ^ message[i];
                    }
                    
                    //Write the data
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

//----------------------

void ADLPCom::parseIncommingByte(unsigned char bytesReturned){
    if(bytesReturned == ACK && sendingMessage){
        if(outgoingFormalities[0] && outgoingFormalities[1]){
            //Sending was succesfull
            if(!sendingMultimessage){
                //Finish up the sending
                outgoingFormalities[0] = false;
                outgoingFormalities[1] = false;
                sendingMessage = false;
                serial.writeByte(EOT);
                if(ADLP_DEBUG) cout<<" Send EOT "<<endl;
            } else {
                //More messages to be send
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
            case 0:  //Not used
               
                break;
            case 1:  //Size of telegram
            {               
                if(ADLP_DEBUG) cout<<"Start new message with size "<<(int)bytesReturned<<endl;
                int oldSize = incommingMessageTemp.size;
                unsigned char  temp[oldSize];
                if(oldSize > 0){
                    //Already data in the message, we need to make it bigger
                    for(int i=0;i<oldSize;i++){
                        temp[i] = incommingMessageTemp.data[i];
                    }
                    free(incommingMessageTemp.data);
                }
                //Set the size of the message, and allocate some data memory
                incommingMessageTemp.size += bytesReturned-8;
                incommingMessageTemp.data = (unsigned char*) malloc((incommingMessageTemp.size+100)*sizeof(unsigned char));
                
                if(oldSize > 0){
                    //If we had some old data, lets copy it back
                    for(int i=0;i<oldSize;i++){
                        incommingMessageTemp.data[i] = temp[i];
                    }
                    multimessageDataIndex = oldSize;
                }
            }
                break;
                
            case 2:  //Destination                
                if(bytesReturned == 1){
                    incommingMessageTemp.fromComputer = true; //How should this be possible?! 
                } else {
                    incommingMessageTemp.fromComputer = false;
                }
                break;  
                
            case 3:  //Destination
                break;   
                
            case 4:  //Instruction
                incommingMessageTemp.instruction = (ARAPInstruction)bytesReturned;
                break;
                
            case 5:  //Message type
                incommingMessageTemp.messageType = (ARAPMessageType)bytesReturned;
                break;
                
            case 6:  //Function suffix
                incommingMessageTemp.functionSuffix = bytesReturned<<8;
                break;
                
            case 7:  //Function suffix 
                incommingMessageTemp.functionSuffix += bytesReturned;
                break;
                
            default:
                //Everything else is data
                incommingMessageTemp.data[multimessageDataIndex+incommingMessageIndex-8] = bytesReturned;
                break;
        }        
        incommingMessageIndex++;
        
    }  
    
    else if(bytesReturned == ENQ){
        //New message is incomming... Return with an acknowledge 
        serial.writeByte(ACK);
        receivingMessage = true;
        incommingMessageIndex = 0; //Reset the index counter
        memset(incommingFormalities,false,5); //reset the formalities handler
        incommingFormalities[0] = true; //Tell that the first formality is met
        multimessageDataIndex = 0;
        incommingMessageTemp.size = 0;        
    }  
    
    else if(bytesReturned == DLE){
        if(!incommingFormalities[1]){
            incommingFormalities[1] = true;            
        }else if(!incommingFormalities[3]) {
            incommingFormalities[3] = true; //Second time DLE is received after message         
        } else {
            //New (multi)message
            if(ADLP_DEBUG) cout<<"----------"<<endl<<"Next message"<<endl;
            incommingMessageIndex = 0; //Reset the index counter
            memset(incommingFormalities,false,5); //reset the formalities handler
            incommingFormalities[0] = true;
            incommingFormalities[1] = true; 
            receivingMessage = true;
        }
    } 
    
    else if(bytesReturned == STXodd || bytesReturned == STXeven){
        incommingFormalities[2] = true;       
    } 
    
    else if(bytesReturned == ETX){ //End of text
        incommingFormalities[4] = true;
    } 
    
    else if(bytesReturned == EOT){ //End of transmission
        if(ADLP_DEBUG){
            printf("Done receiving at msg %i\n",incommingMessageIndex);
            printf("Received instruction: %i\n",incommingMessageTemp.instruction );
        }
        waitingForResponse = false;
        incommingMessageIndex = -1;
        
        incommingMessages.push_back(incommingMessageTemp);
        readMessageCounter ++;
        
        //Clean up the inbox for old messages
        while (incommingMessages.size() > incommingBufferSize){
            incommingMessages.erase(incommingMessages.begin());
        }
        
        incommingMessageTemp = ARAPMessage();
    } 
    
    else if(incommingMessageIndex >= 0 && incommingFormalities[0] && incommingFormalities[1] && incommingFormalities[2] && incommingFormalities[3] && incommingFormalities[4]){
        if(ADLP_DEBUG) printf("Recevied checksum %x\n",bytesReturned);

        //All formalities met, check cheksum (TODO)
        serial.writeByte(ACK); 
        receivingMessage = false;
    }        
}
