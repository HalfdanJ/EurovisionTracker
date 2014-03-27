#include "testApp.h"


void testApp::setup() {
	ofSetVerticalSync(true);
    
    
    
   	thresh.allocate(1920, 1080, OF_IMAGE_GRAYSCALE);
    img.allocate(1920, 1080, OF_IMAGE_COLOR);
    
    
    oscReceiver.setup(8080);
    
    ofSetFrameRate(50);
    ofSetVerticalSync(false);
    
    
#ifdef SIMULATOR
    simulatorFbo.allocate(1920, 1080);
    
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
                simulatorPos[i].x = m.getArgAsFloat(i)*1920;
            }
        }
        if(m.getAddress() == "/position/y"){
            for(int i=0;i<3;i++){
                simulatorPos[i].y = m.getArgAsFloat(i)*1080;
            }
        }
        
    }
    
#ifdef SIMULATOR
    simulatorFbo.begin();
    ofClear(0, 0, 0,255);
    
    for(int i=0;i<3;i++){
        ofPushMatrix();
        ofSetColor(255,255,255);
        
        
        ofTranslate(simulatorPos[i].x+mouseX, simulatorPos[i].y, -simulatorPos[i].z);
        
        ofScale(200, 200);
        ofRect(0, 0, 1, 1);
        
        float sizeX = 0.8;
        float sizeY = 2*sizeX * (4./7.);
        float r = 0.03;
        
        ofTranslate(0.5-(sizeX/2.0), 0.5-(sizeY/2.0));
        ofSetColor(0, 0, 0);
        for(int y=0;y<4;y++){
            for(int x=0;x<7;x++){
                float offset = 0;
                if(x%2 == 1){
                    offset = sizeY*0.5/(4);
                }
                ofCircle(sizeX*x/(7),
                         offset+sizeY*y/(4),
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
    cv::Mat cvImage = ofxCv::toCv(img);
    cv::cvtColor(cvImage, cvBwImage, CV_RGB2GRAY);

    
    for(int i=0;i<trackers.size();i++){
        bool found = trackers[i].update(cvBwImage);
        if(!found){
            trackers.erase(trackers.begin()+i);
        }
    }
    
    for(int i=0;i<trackers.size();i++){
        for(int u=i+1;u<trackers.size();u++){
            ofPoint roiTl1 = ofPoint(trackers[i].roiRect.tl().x,trackers[i].roiRect.tl().y);
            ofPoint p1 = ofPoint(trackers[i].imagePoints[0].x,trackers[i].imagePoints[0].y) + roiTl1;

            ofPoint roiTl2 = ofPoint(trackers[u].roiRect.tl().x,trackers[u].roiRect.tl().y);
            ofPoint p2 = ofPoint(trackers[u].imagePoints[0].x,trackers[u].imagePoints[0].y) + roiTl2;
            
            if(p1.distance(p2) < 2 ){
                trackers.erase(trackers.begin()+u);
                cout<<"Erase "<<u<<"  "<<p1.distance(p2)<<endl;
            }

        }
        
    }
    
    if(trackers.size() < 3){

        cvBwImageClone = cvBwImage.clone();
    
        for(int i=0;i<trackers.size();i++){
            int lineType = 8;
            

            cv::Point rook_points[1][4];
            cv::Point roiTl = trackers[i].roiRect.tl();
            int s = trackers[i].imagePoints.size();
            rook_points[0][3] = cv::Point( trackers[i].imagePoints[0].x, trackers[i].imagePoints[0].y ) + roiTl;
            rook_points[0][1] = cv::Point( trackers[i].imagePoints[s-1].x, trackers[i].imagePoints[s-1].y ) + roiTl;
            rook_points[0][2] = cv::Point( trackers[i].imagePoints[trackers[i].patternDefinition.y-1].x, trackers[i].imagePoints[trackers[i].patternDefinition.y-1].y )+ roiTl;
            rook_points[0][0] = cv::Point( trackers[i].imagePoints[s-trackers[i].patternDefinition.y].x, trackers[i].imagePoints[s-trackers[i].patternDefinition.y].y )+ roiTl;
            
            const cv::Point* ppt[1] = { rook_points[0] };
            int npt[] = { 4 };
            
            cv::fillPoly( cvBwImageClone,
                         ppt,
                         npt,
                         1,
                         cv::Scalar( 255, 255, 255 ),
                         lineType );
        }

        Tracker newTracker;
        bool found = newTracker.update(cvBwImageClone);
        if(found){
            trackers.push_back(newTracker);
        }
    }
    
    
}

void testApp::draw() {
    ofSetColor(255,255,255);
    
    ofPushMatrix();
    ofScale(ofGetWidth(), ofGetHeight());
    img.draw(0, 0,0.5,0.5);
   // ofxCv::drawMat(cvBwImageClone, 0, 0,0.5,0.5);
    
    ofScale(0.5, 0.5);
    
    for(int u=0;u<trackers.size();u++){
        int s = trackers[u].imagePoints.size();
        for (int i=0; i<s; i++){
            float X=trackers[u].imagePoints[i].x/(1920.);
            float Y=trackers[u].imagePoints[i].y/(1080.);
            
            ofSetColor(255, 0, 0);
            
            if(i == 0){
                ofSetColor(0, 255, 0);
            }
            
            if(i == s-1){
                ofSetColor(0, 255, 0);
            }
            if(i == trackers[u].patternDefinition.y-1){
                ofSetColor(0, 255, 0);
            }
            if(i == s-trackers[u].patternDefinition.y){
                ofSetColor(0, 255, 0);
            }
            
            
            ofCircle(X, Y, 0.006);
        }
        
        ofSetColor(0,0,255);
        ofCircle(trackers[u].lastLocation.x/1920.,trackers[u].lastLocation.y/1080., 0.008);
        
        ofSetColor(255, 255, 0);
        ofNoFill();
        ofRect(trackers[u].roiRect.x/1920., trackers[u].roiRect.y/1080., trackers[u].roiRect.width/1920., trackers[u].roiRect.height/1080.);
        ofFill();
    }
    
    
    ofPopMatrix();
    
    ofSetColor(255,255,255);
    
    ofDrawBitmapString(ofToString(ofGetFrameRate()), ofPoint(10,20));
    
}

void testApp::keyPressed(int key){
    
}
