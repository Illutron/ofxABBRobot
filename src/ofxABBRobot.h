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
    
    ARAP_STATUS readStatus(bool async=false);
    
private:
    ARAPMessage responseSyncQuery(ARAPMessage msg);
};