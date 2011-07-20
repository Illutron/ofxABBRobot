#include "arapParser.h"

ARAPMessage ARAPParser::constructMessage(ARAPInstruction instruction, int functionSuffix, unsigned char * data, int datasize){
    ARAPMessage msg;
    msg.size = datasize;
    msg.fromComputer = true;
    msg.instruction = instruction;
    msg.messageType = query;
    msg.functionSuffix = functionSuffix;
    msg.data = (unsigned char*)malloc(datasize * sizeof(unsigned char));
    
    for(int i=0;i<datasize;i++){
        msg.data[i] = data[i];
    }
    
    return msg;    
}

//----------------------

ARAPMessage ARAPParser::constructMoveMessage(ARAP_COORDINATE coord, int velocity, int runSpeed, int functionSuffix, unsigned char moveData){
    unsigned char data[52];
    memset(data, 0, 52);
    
    data[12-8] = 0; //Handpos
    data[13-8] = moveData;
    
    data[14-8] = velocity >> 8;
    data[15-8] = velocity & 255;
    
    data[16-8] = runSpeed >> 8;
    data[17-8] = runSpeed & 255;
    
    data[18-8] = coord.x >> 8;
    data[19-8] = coord.x & 255;
    
    data[20-8] = coord.y >> 8;
    data[21-8] = coord.y & 255;
    
    data[22-8] = coord.z >> 8;
    data[23-8] = coord.z & 255;
    
    data[24-8] = coord.q1 >> 8;
    data[25-8] = coord.q1 & 255;
    
    data[26-8] = coord.q2 >> 8;
    data[27-8] = coord.q2 & 255;
    
    data[28-8] = coord.q3 >> 8;
    data[29-8] = coord.q3 & 255;
    
    data[30-8] = coord.q4 >> 8;
    data[31-8] = coord.q4 & 255;
    
    data[32-8] = 255;
    data[33-8] = 255;
    data[34-8] = 250;
    data[35-8] = 220;
    
    ARAPMessage msg = constructMessage(MOVE, functionSuffix, data, 52);
    return msg;
}

//----------------------


ARAP_STATUS ARAPParser::parseStatusMessage(ARAPMessage msg){
    ARAP_STATUS ret;
    if(msg.instruction != READSTATUS){
        printf("Could not parse status message, since its not that kind of message\n");
        return ret;
    }
    
    unsigned char mode = msg.data[17-8];
    if(mode & 1) {
        ret.mode = STANDBY;
    } else if(mode & 2){
        ret.mode = OPERATION;
    }
    else if(mode & 4){
        ret.mode = EXECUTION;
    }
    else if(mode & 8){
        ret.mode = EMERGENCYSTOP;
    }
    
    ret.location.x = (msg.data[18-8]<<8)+msg.data[19-8];
    ret.location.y = (msg.data[20-8]<<8)+msg.data[21-8];
    ret.location.z = (msg.data[22-8]<<8)+msg.data[23-8];
    ret.location.q1 = (msg.data[24-8]<<8)+msg.data[25-8];
    ret.location.q2 = (msg.data[26-8]<<8)+msg.data[27-8];
    ret.location.q3 = (msg.data[28-8]<<8)+msg.data[29-8];
    ret.location.q4 = (msg.data[30-8]<<8)+msg.data[31-8]; 
    
    return ret;
}