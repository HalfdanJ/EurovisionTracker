    #pragma once

#include "ofMain.h"
#include "ofxCv.h"


class Tracker : public ofxCv::Calibration {
public:
    ofVec2f patternDefinition;
    
    //Last know positions of the markers
    cv::Point2f lastLocation;
    int lowThreshold;
    int highThreshold;
    int thresholdStep;
    float blobMinSize;
    float blobMaxSize;
    float blobMinRoundiness;
    float blobMaxRoundiness;
    int roiSize;

    
    cv::Rect roiRect;
    
    
	vector<cv::Point2f> imagePoints;

    
    Tracker();
    bool update(cv::Mat cvBwImage);
    vector<cv::KeyPoint>  debugTrack(cv::Mat image);
    
    cv::SimpleBlobDetector::Params getTrackerParams();
    
    ofMatrix4x4 modelMatrix;
    
    cv::Size patternSize;
	vector<cv::Point3f> objectPoints;


    bool operator < (const Tracker& otherTracker) const
    {
        return (lastLocation.x < otherTracker.lastLocation.x);
    }
};


