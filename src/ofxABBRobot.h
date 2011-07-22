/////////////////////////////////////////////////////////
//// Addon to communicate with the ABB S3 controller ////
/////////////////////////////////////////////////////////
///// Created by Jonas Jongejan on Illutron.dk 2011 /////
/////////////////////////////////////////////////////////
///////////////////////////////////////////////////////// 

#pragma once
#include "adlpCom.h"
#include "arapParser.h"
#include "arapInstructions.h"

class ofxABBRobot {
public:
    ofxABBRobot();
    
    //This is where the serial communication happens
    ADLPCom * com;
    //This is for creating ARAP messages correctly
    ARAPParser * parser;
    
    
    
    //Tell the robot to start a program
    void startProgram(bool fromStart, int program);
    
    //Tell the robot to stop the current program
    void stopProgram();
    
    //Set the operation mode of the robot 
    void writeMode(ARAP_MODE mode);

    //Read the current status of the robot
    ARAP_STATUS readStatus(bool async=false);
    
    //Download a program (binary)
    ARAP_PROGRAM receiveProgram(int program);

    //Ask the robot to move to a specific coordinate. (or vector of coordinates)
    void move(ARAP_COORDINATE coord, float velocity, float runSpeed, bool absolute=true,  bool robotCoordinates=false);
    void move(vector<ARAP_COORDINATE> coords, float velocity, float runSpeed, bool absolute=true,  bool robotCoordinates=false);

    //Returns if the supplied message is a warning or error
    bool isErrorMessage(ARAPMessage msg);
    bool isWarningMessage(ARAPMessage msg);
    
    //Creates a human readable string from an error or warning message
    string errorMessageToString(ARAPMessage msg);
    string warningMessageToString(ARAPMessage msg);

    
private:
    //Add a message to the queue, and wait for response message
    ARAPMessage responseSyncQuery(ARAPMessage msg);
    
    //Add message to queue and don't wait for any response
    void commandQuery(ARAPMessage msg);

    void sendMoveMessage(ARAP_COORDINATE coord, float velocity, float runSpeed, int functionSuffix, unsigned char moveData, bool last);
    long responseCounter;
};