#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOsc.h"
#include <dispatch/dispatch.h>

#define SIMULATOR 1

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
    void updateTracker();
    
    
    void keyPressed(int key);
    
     ofImage img;
    ofImage thresh;

    
	vector<cv::Point2f> imagePoints[3];
	
    
    
    
    ofVec2f patternDefinitions[3];
    
    vector<cv::KeyPoint> blobKeypoints;
    
    ofxOscReceiver oscReceiver;
    
    
    cv::Mat bwImage;

#ifdef SIMULATOR
    ofFbo simulatorFbo;
    ofVec3f simulatorPos[3];
    

#endif
};
