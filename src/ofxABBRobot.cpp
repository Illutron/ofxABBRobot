#include "ofxABBRobot.h"

ofxABBRobot::ofxABBRobot(){
    parser = new ARAPParser();
    com = new ADLPCom();
}

void ofxABBRobot::update(){
    com->update();
    
    
}

ARAPMessage ofxABBRobot::responseSyncQuery(ARAPMessage msg){
    com->queueMessage(msg);
    bool gotResponse = false;
    ARAPMessage response;
    while (!gotResponse) {
        while(com->available()){
            response = com->readMessage();
            if(response.instruction == msg.instruction){
                cout<<"Got message ################"<<endl;
                gotResponse = true;
                break;
            }
        }
        if(!gotResponse)
            usleep(1000*100);
    }
    return response;
}

ARAP_STATUS ofxABBRobot::readStatus(bool async){
    ARAPMessage msg = parser->constructMessage(READSTATUS, nil, 0);
    
    ARAPMessage response;
    bool gotResponse = false;
    if(!async){
        response = responseSyncQuery(msg);
        gotResponse = true;
    }
    
    ARAP_STATUS ret;
    if(gotResponse){
        //Parse the response
        unsigned char mode = response.data[17-8];
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
        
    }
    return ret;
}


