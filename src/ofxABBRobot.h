#pragma once
#include "adlpCom.h"
#include "arapParser.h"
#include "arapInstructions.h"

class ofxABBRobot {
public:
    ofxABBRobot();
    
    void update();

    ADLPCom * com;
    ARAPParser * parser;

    void startProgram(bool fromStart, int program);
    void stopProgram();
    void writeMode(ARAP_MODE mode);
    ARAP_STATUS readStatus(bool async=false);
    
    bool isErrorMessage(ARAPMessage msg);
    bool isWarningMessage(ARAPMessage msg);
    string errorMessageToString(ARAPMessage msg);
    string warningMessageToString(ARAPMessage msg);
    
private:
    ARAPMessage responseSyncQuery(ARAPMessage msg);
    void commandQuery(ARAPMessage msg);
};