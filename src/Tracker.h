    #pragma once

#include "ofMain.h"
#include "ofxCv.h"


class Tracker {
public:
    ofVec2f patternDefinition;
    
    //Last know positions of the markers
    cv::Point2f lastLocation;
    
    cv::Rect roiRect;
    
    
	vector<cv::Point2f> imagePoints;
    
    
    Tracker();
    bool update(cv::Mat cvBwImage);
};