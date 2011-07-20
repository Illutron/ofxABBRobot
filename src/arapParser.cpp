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