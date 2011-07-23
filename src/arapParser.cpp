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
    ARAP_COORDINATE_RAW raw = coordinateToRaw(coord);
    
    unsigned char data[52];
    memset(data, 0, 52);
    
    data[12-8] = 0; //Handpos
    data[13-8] = moveData;
    
    data[14-8] = velocity >> 8;
    data[15-8] = velocity & 255;
    
    data[16-8] = runSpeed >> 8;
    data[17-8] = runSpeed & 255;
    
    data[18-8] = raw.x >> 8;
    data[19-8] = raw.x & 255;
    
    data[20-8] = raw.y >> 8;
    data[21-8] = raw.y & 255;
    
    data[22-8] = raw.z >> 8;
    data[23-8] = raw.z & 255;
    
    
    data[24-8] = raw.q1 >> 8;
    data[25-8] = raw.q1 & 255;
    
    data[26-8] = raw.q2 >> 8;
    data[27-8] = raw.q2 & 255;
    
    data[28-8] = raw.q3 >> 8;
    data[29-8] = raw.q3 & 255;
    
    data[30-8] = raw.q4 >> 8;
    data[31-8] = raw.q4 & 255;
    
/*    data[32-8] = 255;
    data[33-8] = 255;
    data[34-8] = 250;
    data[35-8] = 220;*/
    
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
    
    ARAP_COORDINATE_RAW raw;
    
    raw.x = (msg.data[18-8]<<8)+msg.data[19-8];
    raw.y = (msg.data[20-8]<<8)+msg.data[21-8];
    raw.z = (msg.data[22-8]<<8)+msg.data[23-8];
    raw.q1 = (msg.data[24-8]<<8)+msg.data[25-8];
    raw.q2 = (msg.data[26-8]<<8)+msg.data[27-8];
    raw.q3 = (msg.data[28-8]<<8)+msg.data[29-8];
    raw.q4 = (msg.data[30-8]<<8)+msg.data[31-8]; 
    
    ret.location = coordinateFromRaw(raw);
    
    return ret;
}

//----------------------




//----------------------

ARAP_COORDINATE ARAPParser::coordinateFromRaw(ARAP_COORDINATE_RAW raw){
    ARAP_COORDINATE ret;
    if(raw.x > 32767)
        raw.x -= 2*32767;
    if(raw.y > 32767)
        raw.y -= 2*32767;
    if(raw.z > 32767)
        raw.z -= 2*32767;
    
    ret.x = raw.x * 0.125;
    ret.y = raw.y * 0.125;
    ret.z = raw.z * 0.125;
    

    
    ret.q1 = raw.q1 * 1.0/16384.0;
    ret.q2 = raw.q2 * 1.0/16384.0;
    ret.q3 = raw.q3 * 1.0/16384.0;
    ret.q4 = raw.q4 * 1.0/16384.0;
    
    return ret;
}

//----------------------


ARAP_COORDINATE_RAW ARAPParser::coordinateToRaw(ARAP_COORDINATE coord){
    ARAP_COORDINATE_RAW ret;
    ret.x = coord.x * 1.0/0.125;
    ret.y = coord.y * 1.0/0.125;
    ret.z = coord.z * 1.0/0.125;
    
    ret.q1 = coord.q1 * 16384.0;
    ret.q2 = coord.q2 * 16384.0;
    ret.q3 = coord.q3 * 16384.0;
    ret.q4 = coord.q4 * 16384.0;
    
    return ret;
}

/*
 Q1 = cos i/2 
 Q2 = nx sin i/2 
 Q3 = ny sin i/2 
 Q4 = nz sin i/2
 
 where n = nx, ny, nz is the orientation of the rotation axis of the wrist coordinate system relative 
 the base coordinate system and i is the rotation angle around n. 
 Before calculating i, the Q1 - 04 should always be multiplied by the scale factor 1/16384 (0,000061).
 
 Help!
*/

