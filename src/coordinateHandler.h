#pragma once
#include "ofxVectorMath.h"
#include "arapInstructions.h"

class CoordinateHandler {    
public:
    CoordinateHandler();
    void setCalibrationCorner(int corner, ARAP_COORDINATE coord);
    ARAP_COORDINATE convertToWorld(float x, float y, float z);
    
    
    ofxVec3f corners[3];
    
    ofxQuaternion dir;
};