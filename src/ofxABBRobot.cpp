#include "ofxABBRobot.h"
#include "ofMain.h"

ofxABBRobot::ofxABBRobot(){
    parser = new ARAPParser();
    com = new ADLPCom();
}

void ofxABBRobot::update(){
    com->update();
}

bool ofxABBRobot::isErrorMessage(ARAPMessage msg){
    if(msg.instruction == 127 && (msg.functionSuffix == 7 || msg.functionSuffix == 9 || msg.functionSuffix == 10 || msg.functionSuffix == 17)){
        return true;
    }
    return false;
}

bool ofxABBRobot::isWarningMessage(ARAPMessage msg){
    if(msg.messageType == warning){
        return true;
    }
    return false;
}

string ofxABBRobot::errorMessageToString(ARAPMessage msg){
    string ret;
    if(isErrorMessage(msg)){
        switch (msg.functionSuffix) {
            case 7:
                ret += "System error";
                break;
            case 9:
                ret += "Vision init error";
                break;
            case 10:
                ret += "Vision test error";
                break;
            case 17:
                ret += "Operational error";
                break;
            default:
                break;
        }
        ret += ofToString((msg.data[8-8]<<8) + msg.data[9-8])+"."+ofToString((msg.data[10-8] << 8) + msg.data[11-8]);
    }
    return ret;
}

string ofxABBRobot::warningMessageToString(ARAPMessage msg){
    string ret;
    if(isWarningMessage(msg)){
        ret += "Warning returned: "+ofToString((msg.data[8-8]<<8) + msg.data[9-8]);
    }
    return ret;
}

ARAPMessage ofxABBRobot::responseSyncQuery(ARAPMessage msg){
    com->queueMessage(msg);
    bool gotResponse = false;
    ARAPMessage response;
    while (!gotResponse) {
        if(com->available()){
            response = com->readMessage();
            if(response.instruction == msg.instruction){
                cout<<"Got message ################"<<endl;
                gotResponse = true;
                com->markRead();
                break;
            }
        }
        if(!gotResponse)
            usleep(1000*100);
    }
    return response;
}

void ofxABBRobot::commandQuery(ARAPMessage msg){
    com->queueMessage(msg);
}


#pragma mark ARAP api
void ofxABBRobot::startProgram(bool fromStart, int program){
    unsigned char data[2];
    data[1] = program & 255;
    data[0] = program - data[1];

    ARAPMessage msg = parser->constructMessage(STARTPROGRAM, fromStart?0:1, data, 2);

    cout<<"&&&&&&&&&&&&&&&& data: "<<(int)data[0]<<"    "<<(int)data[1]<<endl;
    commandQuery(msg);
} 

void ofxABBRobot::stopProgram(){
    commandQuery(parser->constructMessage(STOPPROGRAM));
}
    
void ofxABBRobot::writeMode(ARAP_MODE mode){
    if(mode > 3){
        cout<<"Wrong input mode in writeMode()"<<endl;
        return;
    }
    ARAPMessage msg = parser->constructMessage(WRITEMODE, mode);    
    commandQuery(msg);
}

ARAP_STATUS ofxABBRobot::readStatus(bool async){
    ARAPMessage msg = parser->constructMessage(READSTATUS);
    
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


