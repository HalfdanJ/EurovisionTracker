//
//  Tracker.cpp
//  Eurovision2
//
//  Created by Jonas Jongejan on 19/03/14.
//
//

#include "Tracker.h"


Tracker::Tracker(){
    patternDefinition = ofVec2f(11,4);
    thresholdStep = 9;
    
    
	load("simulator.yml");
    setPatternSize(4, 11);
    setPatternType(ofxCv::ASYMMETRIC_CIRCLES_GRID);
    setSquareSize(20);
    
    patternSize = getPatternSize();


}


cv::SimpleBlobDetector::Params Tracker::getTrackerParams(){
    cv::SimpleBlobDetector::Params params = cv::SimpleBlobDetector::Params();
    
    //            cout<<params.thresholdStep<<endl;
    params.minThreshold = lowThreshold;
    params.maxThreshold = highThreshold;
    params.thresholdStep = thresholdStep;
    params.minArea = 200;
    params.filterByConvexity = false;
    params.filterByCircularity = false;
    return  params;
}

vector<cv::KeyPoint>  Tracker::debugTrack(cv::Mat image){
    vector<cv::KeyPoint> keypoints;
    
    cv::SimpleBlobDetector * blobDetector =  new cv::SimpleBlobDetector(getTrackerParams());
    
    blobDetector->create("SimpleBlob");
    blobDetector->detect(image, keypoints);
    
    return keypoints;
}

bool Tracker::update(cv::Mat img){
    bool found=false;
    int flags = cv::CALIB_CB_ASYMMETRIC_GRID;
    flags += cv::CALIB_CB_CLUSTERING;
    
    roiRect = cv::Rect(0, 0, 1920, 1080);
    if(lastLocation.x != 0 && lastLocation.y != 0){
        int roiSize = 400;
        roiRect = cv::Rect(
                           ofClamp(lastLocation.x-roiSize*0.5,0,1920-roiSize),
                           ofClamp(lastLocation.y-roiSize*0.5,0,1080-roiSize),
                           roiSize,
                           roiSize);
    }
    cv::Mat cvImageRoi = img(roiRect);
    
    
    patternSize = cv::Size(patternDefinition.y, patternDefinition.x);
    objectPoints = Calibration::createObjectPoints(patternSize, 2.5, ofxCv::ASYMMETRIC_CIRCLES_GRID);


    cv::SimpleBlobDetector * blobDectetor =  new cv::SimpleBlobDetector(getTrackerParams());
    
    found = findCirclesGrid(cvImageRoi, patternSize, imagePoints, flags,  blobDectetor);
    
    if(found) {
        cv::Mat cameraMatrix = getDistortedIntrinsics().getCameraMatrix();
        //cameraMatrix = (cv::Mat_<double>(3,3) << 930, 0, 960, 0, 930, 540, 0, 0, 1);
        
        cv::Mat rvec, tvec;
        
        
        cv::Point2f tl = roiRect.tl();
        for(int i=0;i<imagePoints.size();i++){
            imagePoints[i] += tl;
        }
        cv::solvePnP(cv::Mat(objectPoints), cv::Mat(imagePoints), cameraMatrix, getDistCoeffs(), rvec, tvec);
        modelMatrix = ofxCv::makeMatrix(rvec, tvec);
        
        int c = patternDefinition.y * patternDefinition.x;
        lastLocation = imagePoints[c/2];

    } else {
        //Not found
        lastLocation.x = 0;
        lastLocation.y = 0;
    }
    
    
    return found;
    
    
    /*
     
     int flags = cv::CALIB_CB_ASYMMETRIC_GRID;
     flags += cv::CALIB_CB_CLUSTERING;
     
     
     
     roiRect = cv::Rect(0, 0, 1920, 1080);
     if(lastLocation.x != 0 && lastLocation.y != 0){
     int roiSize = 600;
     roiRect = cv::Rect(
     ofClamp(lastLocation.x-roiSize*0.5,0,1920-roiSize),
     ofClamp(lastLocation.y-roiSize*0.5,0,1080-roiSize),
     roiSize,
     roiSize);
     }
     cv::Mat cvImageRoi = cvBwImage(roiRect);
     
     
     
     cv::Size patternSize = cv::Size(patternDefinition.y, patternDefinition.x);
     
     float squareSize = 2.5;
     vector<cv::Point3f> corners;
     for(int i = 0; i < patternSize.height; i++)
     for(int j = 0; j < patternSize.width; j++)
     corners.push_back(cv::Point3f(float(((2 * j) + (i % 2)) * squareSize), float(i * squareSize), 0));
     
     
     vector<cv::Point3f> objectPoints = corners;
     
     
     
     cv::SimpleBlobDetector * blobDetector =  new cv::SimpleBlobDetector(getTrackerParams());
     
     
     bool found = cv::findCirclesGrid( cvImageRoi, patternSize, imagePoints, flags,  blobDetector);
     
     if(found){
     //        cout<<i<<"  found"<<endl;
     cv::Mat cameraMatrix = (cv::Mat_<double>(3,3) << 930, 0, 960, 0, 930, 540, 0, 0, 1);
        cv::Mat distMatrix = (cv::Mat_<double>(5,1) << 0,0,0,0,0);
     
        //            cout<<cameraMatrix<<endl;
        cv::Mat rvec, tvec;
        solvePnP(cv::Mat(objectPoints), cv::Mat(imagePoints), cameraMatrix, distMatrix, rvec, tvec);
        //            cout<<tvec<<"  "<<rvec<<endl;
        
        
        
        int c = patternDefinition.y * patternDefinition.x;
        
        
        lastLocation = imagePoints[c/2] + cv::Point2f(roiRect.tl().x, roiRect.tl().y);
        
    } else {
        //Not found
        lastLocation.x = 0;
        lastLocation.y = 0;
    }
    
    return found;*/
}