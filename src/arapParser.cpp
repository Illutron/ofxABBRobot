#include "arapParser.h"

ARAPMessage ARAPParser::constructMessage(ARAPInstruction instruction, unsigned char * data, int datasize){
    ARAPMessage msg;
    msg.size = datasize + 8;
    msg.fromComputer = true;
    msg.instruction = instruction;
    msg.messageType = query;
    msg.data = data;
    
    return msg;    
}