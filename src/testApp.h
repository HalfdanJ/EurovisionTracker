#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOsc.h"
#include "Tracker.h"
#include "ofxXmlSettings.h"

#include <dispatch/dispatch.h>

#define SIMULATOR 1
//#define BLACKMAGIC 1
//#define VIDEO 1

#ifdef BLACKMAGIC
#include "ofxBlackMagic.h"
#endif


class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
    void updateTracker();
    
    
    void keyPressed(int key);
    void keyReleased(int key);
    
    ofVec3f lastMouse;
    void mouseMoved( int x, int y );
    void mouseDragged( int x, int y, int button );

    ofxXmlSettings settings;
    ofxOscReceiver oscReceiver;
    
    vector<Tracker> trackers;
    
    vector<cv::KeyPoint> blobKeypoints;
    
    
    cv::Mat cvBwImage;
    cv::Mat cvBwImageClone;
    
    bool trackerReady;
    bool setThreshold;
    
    int threshold;
    vector<cv::KeyPoint> blobs;
    
    Tracker debugTracker;

#ifdef SIMULATOR
    ofFbo simulatorFbo;
    ofVec3f simulatorPos[3];
    ofImage img;

#endif
    
#ifdef VIDEO
    ofVideoPlayer img;
#endif
    
#ifdef BLACKMAGIC
    ofxBlackMagic cam;
    
    void exit() {
		cam.close();
	}
#endif
};
