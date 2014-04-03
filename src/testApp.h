#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOsc.h"
#include "Tracker.h"
#include "ofxXmlSettings.h"
#include "ofxTimeMeasurements.h"
#include "ofxRemoteUIServer.h"
#include "ofxSyphon.h"
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
    void updateTracker();
    void updateSimulator();

	void draw();
    void drawBox(int box);
    
    
    void keyPressed(int key);
    void keyReleased(int key);
    
    ofVec3f lastMouse;
    void mouseMoved( int x, int y );
    void mouseDragged( int x, int y, int button );

    ofxXmlSettings settings;
    ofxOscReceiver oscReceiver;
    ofxSyphonClient syphon;
    
    vector<Tracker> trackers;
    vector<Tracker> unusedTrackers;
    
    cv::Mat cvBwImage;

    int threshold;
    float blobMinSize;
    float blobMaxSize;
    float blobMinRoundiness;
    float blobMaxRoundiness;
    int roiSize;
    int numTrackers;
    
    
    bool setThreshold;
    
    ofFbo mask[3];
    ofFbo boxContent[3];
    ofFbo composed[3];
    
    ofLight pointLight, pointLight2, pointLight3;
    
    ofMaterial material;
    
    
    //Debug
    bool debug;
    Tracker debugTracker;
    vector<cv::KeyPoint> blobs;
    

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
