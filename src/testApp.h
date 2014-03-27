#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOsc.h"
#include "Tracker.h"
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
    
    
    vector<Tracker> trackers;
    
    vector<cv::KeyPoint> blobKeypoints;
    
    ofxOscReceiver oscReceiver;
    
    
    cv::Mat cvBwImage;
    cv::Mat cvBwImageClone;
    
#ifdef SIMULATOR
    ofFbo simulatorFbo;
    ofVec3f simulatorPos[3];
    
    
#endif
};
