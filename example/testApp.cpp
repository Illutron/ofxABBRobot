#include "testApp.h"



//--------------------------------------------------------------
void testApp::setup(){	 

	ofSetVerticalSync(true);
	ofBackground(0,0,0);		
	
	
	//----------------------------------- 
	font.loadFont("DIN.otf",14);
}

//--------------------------------------------------------------
void testApp::update(){
	adlp.update();
}

//--------------------------------------------------------------
void testApp::draw(){

}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){ 
	if(key == 'v'){
        unsigned char data[4];
        data[0] = 0;
        data[1] = 1;
        data[2] = 0;
        data[3] = 2;
		adlp.sendMessage((ADLPInstruction)0x02, data, 4);
    
    } if(key == 's'){
        adlp.sendMessage((ADLPInstruction)stopProgram, nil, 0);
    }
    if(key == 'i'){
        adlp.sendMessage((ADLPInstruction)0x15, nil, 0);
    }
    if(key == 'd'){
        unsigned char data[4];
        data[0] = 0;
        data[1] = 1;
        data[3] = 0;
        data[4] = 1;
        adlp.sendMessage((ADLPInstruction)0x1D, data, 4);
    }
    
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){ 
	
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

