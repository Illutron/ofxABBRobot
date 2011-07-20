#include "ofxABBRobot.h"
#include "ofMain.h"

string errorString(int errorNum){
    switch (errorNum) {
        case 0:
            return "Not specified";
            break;
        case 1:
            return "Program running";
            break;
        case 2:
            return "Wrong function suffix";
            break;
        case 3:
            return "Wrong program number";
            break;
        case 4:
            return "Wrong BLOCK number";
            break;
        case 5:
            return "Wrong key mode";
            break;
        case 6:
            return "Wrong operation mode";
            break;
        case 7:
            return "Non existent program number";
            break;
        case 8:
            return "Not specified";
            break;
        case 9:
            return "Block is to big";
            break;
        case 10:
            return "No more space left";
            break;
        case 11:
            return "Wrong function code";
            break;
        case 12:
            return "Floppydisc error";
            break;
        case 13:
            return "Stop, emergency stop";
            break;
        case 14:
            return "Not specified";
            break;
        case 15:
            return "Outside the work area or a lack of start point";
            break;
        case 16:
            return "Workstop";
            break;
        case 17:
            return "Dependency is NULL";
            break;
        case 18:
            return "No welding robot";
            break;
            
        case 30:
            return "Incorrect checksum in the configuration data";
            break;
        case 31:
            return "Not possible to proceed";
            break;
        case 32:
            return "Incorrect welding data release";
            break;
        case 33:
            return "LOCATION reading a register, with 20-29 active TCP is not possible";
            break;
        case 34:
            return "Move the robot with an active TCP 20-29 is not possible";
            break;
        case 35:
            return "Change the RESOLVER DATA not possible";
            break;
        case 36:
            return "ANALOG OUTPUTS value out of range";
            break;
        case 99:
            return "Timeout";
            break;
        case 255:
            return "Did not get the whole telegram content";
            break;
            
        default:
            return "Wrong error code";
            break;
    }
}



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
        ret += "Warning returned: "+ofToString((msg.data[8-8]<<8) + msg.data[9-8])+"  "+errorString((msg.data[8-8]<<8)+ msg.data[9-8]);
    }
    return ret;
}

ARAPMessage ofxABBRobot::responseSyncQuery(ARAPMessage msg){
    responseCounter = com->readMessageCounter;
    com->queueMessage(msg);
    bool gotResponse = false;
    ARAPMessage response;
    while (!gotResponse) {
        vector<ARAPMessage> messages = com->readMessagesAfterCount(responseCounter);
        responseCounter = com->readMessageCounter;
        for(int i=0;i<messages.size();i++){
            if(messages[i].instruction == msg.instruction){
                response = messages[i];
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

void ofxABBRobot::commandQuery(ARAPMessage msg){
    com->queueMessage(msg);
}


#pragma mark ARAP api

void ofxABBRobot::startProgram(bool fromStart, int program){
    unsigned char data[2];
    data[1] = program & 255;
    data[0] = program - data[1];
    ARAPMessage msg = parser->constructMessage(STARTPROGRAM, fromStart?0:1, data, 2);
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
        
        ret.location.x = (response.data[18-8]<<8)+response.data[19-8];
        ret.location.y = (response.data[20-8]<<8)+response.data[21-8];
        ret.location.z = (response.data[22-8]<<8)+response.data[23-8];
        ret.location.q1 = (response.data[24-8]<<8)+response.data[25-8];
        ret.location.q2 = (response.data[26-8]<<8)+response.data[27-8];
        ret.location.q3 = (response.data[28-8]<<8)+response.data[29-8];
        ret.location.q4 = (response.data[30-8]<<8)+response.data[31-8];        
    }
    return ret;
}

ARAP_PROGRAM ofxABBRobot::receiveProgram(int program){
    unsigned char data[4];
    data[1] = program & 255;
    data[0] = program - data[1];
    data[2] = 0;
    data[3] = 0;
    ARAPMessage msg = parser->constructMessage(RECEIVEPROGRAM, 1, data, 4);
    
    ARAPMessage response = responseSyncQuery(msg);
    ARAP_PROGRAM ret;
    ret.data = (unsigned char*)malloc((response.size - 4) * sizeof(unsigned char));
    for(int i=0;i<(response.size - 4);i++){
        ret.data[i] = response.data[4+i];
    }
    printf("Error / program number: %i %i",response.data[0], response.data[1]);
    //memcpy((void*)ret.data, response.data+4, (response.size - 4) * sizeof(unsigned char));
    cout<<"--------"<<endl<<endl;
    for(int i=0;i<(response.size - 4);i++){
        //        if(ret.data[i] == 0){
        //            ret.data[i] = ' ';
        //        }
        printf("%i\n",ret.data[i]);
    }
    //    cout<<ret.data<<endl;
    return ret;
}


void ofxABBRobot::move(ARAP_COORDINATE coord, int velocity, int runSpeed, bool absolute){
    unsigned char data[52];
    memset(data, 0, 52);
    
    data[13-8] = 0; //Move data
    
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
    data[34-8] = 255;
    data[35-8] = 114;
    //    ARAPMessage msg = parser->constructMessage(MOVE, absolute?1:0, data, 52);
    ARAPMessage msg = parser->constructMessage(MOVE, 0, data, 52);
    commandQuery(msg);
    
    
}




