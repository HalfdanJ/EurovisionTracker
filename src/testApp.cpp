#include "testApp.h"

using namespace ofxCv;
using namespace cv;

void testApp::setup() {
	ofSetVerticalSync(true);
    
    
    
    
    
   	thresh.allocate(1920*0.5, 1080*0.5, OF_IMAGE_GRAYSCALE);
    img.allocate(1920*0.5, 1080*0.5, OF_IMAGE_COLOR);
    
    for(int i=0;i<3;i++){
        patternDefinitions[i] = ofVec2f(7,4);
    }
    
    oscReceiver.setup(8080);
    
    ofSetFrameRate(25);
    
    
#ifdef SIMULATOR
    simulatorFbo.allocate(1920*0.5, 1080*0.5);
    
    simulatorPos[0].x = 100;
    simulatorPos[0].y = 200;
    
    simulatorPos[1].x = 400;
    simulatorPos[1].y = 200;
    
    
    simulatorPos[2].x = 750;
    simulatorPos[2].y = 200;
    simulatorPos[2].z = 100;
    
#endif
}

void testApp::update() {
    
    while(oscReceiver.hasWaitingMessages()){
        ofxOscMessage m;
        oscReceiver.getNextMessage(&m);
        
        if(m.getAddress() == "/position/x"){
            for(int i=0;i<3;i++){
                simulatorPos[i].x = m.getArgAsFloat(i)*1920*0.5;
            }
        }
        if(m.getAddress() == "/position/y"){
            for(int i=0;i<3;i++){
                simulatorPos[i].y = m.getArgAsFloat(i)*1080*0.5;
            }
        }
        
    }
    
#ifdef SIMULATOR
    simulatorFbo.begin();
    ofClear(0, 0, 0,255);
    
    for(int i=0;i<3;i++){
        ofPushMatrix();
        ofSetColor(255,255,255);
        
        
        ofTranslate(simulatorPos[i].x, simulatorPos[i].y, -simulatorPos[i].z);
        
        ofScale(200, 200);
        ofRect(0, 0, 1, 1);
        
        float sizeX = 0.8;
        float sizeY = 2*sizeX * (patternDefinitions[i].y/patternDefinitions[i].x);
        float r = 0.03;
        
        ofTranslate(0.5-(sizeX/2.0), 0.5-(sizeY/2.0));
        ofSetColor(0, 0, 0);
        for(int y=0;y<patternDefinitions[i].y;y++){
            for(int x=0;x<patternDefinitions[i].x;x++){
                float offset = 0;
                if(x%2 == 1){
                    offset = sizeY*0.5/(patternDefinitions[i].y);
                }
                ofCircle(sizeX*x/(patternDefinitions[i].x),
                         offset+sizeY*y/(patternDefinitions[i].y),
                         r);
            }
        }
        
        
        ofPopMatrix();
    }
    simulatorFbo.end();
    
    ofPixels pixels;
    simulatorFbo.readToPixels(pixels);
    
    img.setFromPixels(pixels);
    
#endif
    
    updateTracker();
    
    
    
}

void testApp::updateTracker(){
    
    
    
    Mat cvImage = toCv(img);
    //    cv::resize(cvImage, bwImage, cv::Size(), 0.5, 0.5, INTER_NEAREST);
    cv::Rect region_of_interest = cv::Rect(400, 0, 300, 540);
    Mat image_roi = cvImage(region_of_interest);
    
    
    cv::cvtColor(image_roi, bwImage, CV_RGB2GRAY);
    
    
cv:SimpleBlobDetector::Params params = cv::SimpleBlobDetector::Params();
    
    //            cout<<params.thresholdStep<<endl;
    params.minThreshold = 70;
    params.maxThreshold = 90;
    params.thresholdStep = 19;
    
    // params.filterByInertia = false;
    // params.filterByColor = false;
    /*
     params.filterByArea = false;
     params.filterByCircularity = false;
     params.filterByColor = false;
     params.filterByConvexity = false;
     params.filterByInertia = false;
     */
    
    //            params.minArea = 50;
    //          params.maxArea = 5000;
    
    /* blobDetector->create("BlobDetector");
     blobKeypoints.clear();
     blobDetector->detect(bwImage, blobKeypoints);
     
     // extract the x y coordinates of the keypoints:
     
     cout<<"Blobs "<<blobKeypoints.size()<<endl;
     */
    
    int flags = CALIB_CB_ASYMMETRIC_GRID;
    flags += CALIB_CB_CLUSTERING;
    
    
    for(int i=0;i<3;i++){
        //  dispatch_async(dispatch_get_global_queue( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^(void){
        
        cv::Size patternSize = cv::Size(patternDefinitions[i].y, patternDefinitions[i].x);
        
        float squareSize = 2.5;
        vector<Point3f> corners;
        for(int i = 0; i < patternSize.height; i++)
            for(int j = 0; j < patternSize.width; j++)
                corners.push_back(Point3f(float(((2 * j) + (i % 2)) * squareSize), float(i * squareSize), 0));
        
        
        vector<cv::Point3f> objectPoints = corners;
        
        
        
        cv::SimpleBlobDetector * blobDetector =  new SimpleBlobDetector(params);
        
        bool found = cv::findCirclesGrid( bwImage, patternSize, imagePoints[i], flags,  blobDetector);
        
        if(found){
            cout<<i<<"  found"<<endl;
            Mat cameraMatrix = (Mat_<double>(3,3) << 930, 0, 960, 0, 930, 540, 0, 0, 1);
            Mat distMatrix = (Mat_<double>(5,1) << 0,0,0,0,0);
            
            //            cout<<cameraMatrix<<endl;
			Mat rvec, tvec;
			solvePnP(Mat(objectPoints), Mat(imagePoints[i]), cameraMatrix, distMatrix, rvec, tvec);
            //            cout<<tvec<<"  "<<rvec<<endl;
            
            int lineType = 8;
            
            /** Create some points */
            cv::Point rook_points[1][4];
            int s = imagePoints[i].size();
            
            rook_points[0][0] = cv::Point( imagePoints[i][0].x, imagePoints[i][0].y );
            rook_points[0][1] = cv::Point( imagePoints[i][s-1].x, imagePoints[i][s-1].y );
            rook_points[0][2] = cv::Point( imagePoints[i][patternDefinitions[i].y-1].x, imagePoints[i][patternDefinitions[i].y-1].y );
            rook_points[0][3] = cv::Point( imagePoints[i][s-patternDefinitions[i].y].x, imagePoints[i][s-patternDefinitions[i].y].y );
            
            const cv::Point* ppt[1] = { rook_points[0] };
            int npt[] = { 4 };
            
            cv::fillPoly( bwImage,
                         ppt,
                         npt,
                         1,
                         Scalar( 255, 255, 255 ),
                         lineType );
        }
        //  });
        
        //cout<<found<<" "<<imagePoints[i].size()<<endl;
    }
    
}

void testApp::draw() {
    ofSetColor(255,255,255);
    
    ofPushMatrix();
    ofScale(ofGetWidth(), ofGetHeight());
    img.draw(0, 0,0.5,0.5);
    
    for(int u=0;u<3;u++){
        int s = imagePoints[u].size();
        for (int i=0; i<s; i++){
            float X=0.5*imagePoints[u][i].x/(1920.*0.5);
            float Y=0.5*imagePoints[u][i].y/(1080.*0.5);
            
            ofSetColor(255, 0, 0);
            
            if(i == 0){
                ofSetColor(0, 255, 0);
            }
            
            if(i == s-1){
                ofSetColor(0, 255, 0);
            }
            if(i == patternDefinitions[0].y-1){
                ofSetColor(0, 255, 0);
            }
            if(i == s-patternDefinitions[0].y){
                ofSetColor(0, 255, 0);
            }
            
            
            ofCircle(X, Y, 0.003);
        }
    }
    
    
    ofPopMatrix();
    
    ofSetColor(255,255,255);
    
    ofDrawBitmapString(ofToString(ofGetFrameRate()), ofPoint(10,20));
    
}

void testApp::keyPressed(int key){
    
}
