#include "coordinateHandler.h"

CoordinateHandler::CoordinateHandler(){
    
}

void CoordinateHandler::setCalibrationCorner(int corner, ARAP_COORDINATE coord){
    corners[corner].x = coord.x;
    corners[corner].y = coord.y;
    corners[corner].z = coord.z;
    
    if(corner == 0){
        dir = ofxQuaternion(coord.q1, coord.q2, coord.q3, coord.q4);
    }
}

ARAP_COORDINATE CoordinateHandler::convertToWorld(float x, float y, float z){
    ARAP_COORDINATE ret;
    
    ofxVec3f xdir = (corners[1] - corners[0]).normalized();
    ofxVec3f ydir = (corners[2] - corners[0]).normalized();
    
    ofxVec3f zdir = ydir.crossed(xdir);
    
    ofxVec3f v = xdir*x + ydir*y + zdir*z;
    
    ret.x = corners[0].x + v.x;
    ret.y = corners[0].y + v.y;
    ret.z = corners[0].z + v.z;
    
    ret.q1 = dir[0];
    ret.q2 = dir[1];
    ret.q3 = dir[2];
    ret.q4 = dir[3];

    return ret;
}