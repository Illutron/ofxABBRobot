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

//----------------------







//----------------------


ofxABBRobot::ofxABBRobot(){
    //Construct the sub objects
    parser = new ARAPParser();
    com = new ADLPCom();
}

//----------------------

bool ofxABBRobot::isErrorMessage(ARAPMessage msg){
    if(msg.instruction == 127 && (msg.functionSuffix == 7 || msg.functionSuffix == 9 || msg.functionSuffix == 10 || msg.functionSuffix == 17)){
        return true;
    }
    return false;
}

//----------------------


bool ofxABBRobot::isWarningMessage(ARAPMessage msg){
    if(msg.messageType == warning){
        return true;
    }
    return false;
}

//----------------------

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

//----------------------

string ofxABBRobot::warningMessageToString(ARAPMessage msg){
    string ret;
    if(isWarningMessage(msg)){
        ret += "Warning returned: "+ofToString((msg.data[8-8]<<8) + msg.data[9-8])+"  "+errorString((msg.data[8-8]<<8)+ msg.data[9-8]);
    }
    return ret;
}


//----------------------
//----------------------

#pragma mark ARAP api

void ofxABBRobot::startProgram(bool fromStart, int program){
    unsigned char data[2];
    data[1] = program & 255;
    data[0] = program - data[1];
    ARAPMessage msg = parser->constructMessage(STARTPROGRAM, fromStart?0:1, data, 2);
    commandQuery(msg);
} 

//----------------------

void ofxABBRobot::stopProgram(){
    commandQuery(parser->constructMessage(STOPPROGRAM));
}

//----------------------

void ofxABBRobot::writeMode(ARAP_MODE mode){
    if(mode > 3){
        cout<<"Wrong input mode in writeMode()"<<endl;
        return;
    }
    ARAPMessage msg = parser->constructMessage(WRITEMODE, mode);    
    commandQuery(msg);
}

//----------------------

ARAP_STATUS ofxABBRobot::readStatus(bool async){
    ARAPMessage msg = parser->constructMessage(READSTATUS);
    
    ARAPMessage response;
    bool gotResponse = false;
    if(!async){
        response = responseSyncQuery(msg);
        gotResponse = true;
    } else {
        cout<<"Error: Async not implemented yet"<<endl;
    }
    
    ARAP_STATUS ret;
    if(gotResponse){
        //Parse the response
        ret = parser->parseStatusMessage(response);
    }
    return ret;
}

//----------------------

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
    cout<<"--------"<<endl<<endl;
    for(int i=0;i<(response.size - 4);i++){
        printf("%i\n",ret.data[i]);
    }
    return ret;
}

//----------------------

void ofxABBRobot::sendMoveMessage(ARAP_COORDINATE coord, float velocity, float runSpeed, int functionSuffix, unsigned char moveData, bool last){
    int velocityRaw = velocity * 1.0/0.125;
    int runSpeedRaw = runSpeed * 8192;
    ARAPMessage msg = parser->constructMoveMessage(coord, velocityRaw, runSpeedRaw, functionSuffix, moveData);
    
    if(!last){
        //Create it as multimessage. Not sure if it helps anything.
        msg.messageType = multifrompc;
    }
    commandQuery(msg);
}

//----------------------

void ofxABBRobot::move(ARAP_COORDINATE coord, float velocity, float runSpeed, bool absolute, bool robotCoordinates){
    vector<ARAP_COORDINATE> v;
    v.push_back(coord);
    move(v, velocity, runSpeed, absolute, robotCoordinates);
}

//----------------------

void ofxABBRobot::move(vector<ARAP_COORDINATE> coords, float velocity, float runSpeed, bool absolute,  bool robotCoordinates){
    unsigned char robotBit = 0;
    if(robotCoordinates){
        robotBit = 4;
    }
    
    
    //Send start point (required by protocol)
    sendMoveMessage(coords[0], velocity, runSpeed, 2,2+robotBit, true);
    
    for(int i=0;i<coords.size();i++){
        sendMoveMessage(coords[i], velocity, runSpeed, 0, 0+robotBit, true); 
    }
    
    //Send stop (required by protocol)
    ARAP_COORDINATE endpoint = coords[coords.size()-1];
    sendMoveMessage(endpoint, velocity, runSpeed, 3, 2+robotBit, true);
}

#pragma mark private calls

//----------------------

ARAPMessage ofxABBRobot::responseSyncQuery(ARAPMessage msg){
    //Store the current message counter before we send the request
    responseCounter = com->readMessageCounter;
    com->queueMessage(msg); //Sends the command
    
    bool gotResponse = false;
    ARAPMessage response;
    while (!gotResponse) {
        //Check whats new in the inbox 
        vector<ARAPMessage> messages = com->readMessagesAfterCount(responseCounter);
        //Update the message counter so we don't see the same messages next time
        responseCounter = com->readMessageCounter;
        //Loop through the messages to see if any match our instruction
        for(int i=0;i<messages.size();i++){
            if(messages[i].instruction == msg.instruction){
                response = messages[i];
                gotResponse = true;
                break;
            }
        }
        //Lets sleep a bit before checking again (to not hang on the lock)
        if(!gotResponse)
            usleep(1000*100);
    }
    return response;
}

//----------------------

void ofxABBRobot::commandQuery(ARAPMessage msg){
    com->queueMessage(msg);
}




