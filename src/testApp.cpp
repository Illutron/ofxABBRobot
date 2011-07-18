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
		adlp.sendMessage();
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

