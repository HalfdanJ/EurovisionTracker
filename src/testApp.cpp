#include "testApp.h"

int patternWidth = 5;
int patternHeight = 2;


void testApp::setup() {
    
    
    
    
    
    oscReceiver.setup(8080);
    
    ofSetFrameRate(50);
    ofSetVerticalSync(true);
    
    trackerReady = true;
    
    settings.loadFile("settings.xml");
    threshold = settings.getValue("threshold", 100);
    
    
#ifdef SIMULATOR
    simulatorFbo.allocate(1920, 1080);
    
    simulatorPos[0].x = 100;
    simulatorPos[0].y = 200;
    
    simulatorPos[1].x = 300;
    simulatorPos[1].y = 200;
    
    
    simulatorPos[2].x = 750;
    simulatorPos[2].y = 200;
    simulatorPos[2].z = 100;
    img.allocate(1920, 1080, OF_IMAGE_COLOR);
    
#endif
    
#ifdef BLACKMAGIC
    cam.setup(1920, 1080, 30);
    
#endif
    
    
#ifdef VIDEO
    img.loadMovie("test.mov");
    img.play();
#endif
    
    
}

void testApp::update() {
    
#ifdef SIMULATOR
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
    
    
    simulatorFbo.begin();
    ofClear(0, 0, 0,255);
    
    for(int i=0;i<3;i++){
        ofPushMatrix();
        ofSetColor(255,255,255);
        
        
        ofTranslate(simulatorPos[i].x+mouseX, simulatorPos[i].y, -simulatorPos[i].z+mouseY);
        
        ofScale(200, 200);
        ofRect(0, 0, 1, 1);
        
        float sizeX = 0.8;
        float sizeY = 2*sizeX * (1.0*patternHeight/patternWidth);
        float r = 0.10;
        
        ofTranslate(0.5-(sizeX/2.0)+0.05, 0.5-(sizeY/2.0)+0.05);
        ofSetColor(0, 0, 0);
        for(int y=0;y<patternHeight;y++){
            for(int x=0;x<patternWidth;x++){
                float offset = 0;
                if(x%2 == 1){
                    offset = sizeY*0.5/(patternHeight);
                }
                ofCircle(sizeX*x/(patternWidth),
                         offset+sizeY*y/(patternHeight),
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
    
    
    bool update = true;
    
#ifdef VIDEO
    img.update();
#endif
    
#ifdef BLACKMAGIC
    update = cam.update();
#endif
    
    if(update)
        updateTracker();
    
    
    
}

void testApp::updateTracker(){
#ifdef BLACKMAGIC
    cvBwImage = ofxCv::toCv(cam.getGrayPixels());
#else
    cv::Mat cvImage = ofxCv::toCv(img);
    cv::cvtColor(cvImage, cvBwImage, CV_RGB2GRAY);
#endif
    
    Tracker debugTracker;
    debugTracker.lowThreshold = threshold;
    debugTracker.highThreshold = threshold+10;
    blobs =debugTracker.debugTrack(cvBwImage);
    
    if(trackerReady){
        trackerReady = false;
        
        dispatch_group_t group = dispatch_group_create();
        
        bool trackerFound[3] = {false,false,false}, *trackerFoundPtr;
        trackerFoundPtr = trackerFound;
        
        
        
        for(int i=0;i<trackers.size();i++){
            trackers[i].lowThreshold = threshold;
            trackers[i].highThreshold = threshold+10;
            
            bool found = trackers[i].update(cvBwImage);
            if(found){
                trackerFoundPtr[i] = true;
            }
        }
        
        //  dispatch_group_notify(group,dispatch_get_main_queue(), ^ {
        for(int i=0;i<trackers.size();i++){
            if(!trackerFoundPtr[i]){
                cout<<"Delete "<<i<<endl;
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
                }
                
            }
            
        }
        
        if(trackers.size() < 2){
            
            cvBwImageClone = cvBwImage.clone();
            
            for(int i=0;i<trackers.size();i++){
                int lineType = 8;
                
                
                cv::Point rook_points[1][4];
                cv::Point roiTl = trackers[i].roiRect.tl();
                int s = trackers[i].imagePoints.size();
                if(s>0){
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
            }
            
            Tracker newTracker;
            newTracker.lowThreshold = threshold;
            newTracker.highThreshold = threshold+10;
            newTracker.patternDefinition.x = patternWidth;
            newTracker.patternDefinition.y = patternHeight;
            bool found = newTracker.update(cvBwImageClone);
            if(found){
                trackers.push_back(newTracker);
            }
        }
        
        trackerReady = true;
        //   });
    }
    
    
}

void testApp::draw() {
    
    
    ofSetColor(255,255,255);
    
    
    
    if(setThreshold){
        cv::threshold(cvBwImage, cvBwImage,threshold, 255, cv::THRESH_BINARY);
        ofxCv::drawMat(cvBwImage,0,0,ofGetWidth(),ofGetHeight());
        
        ofSetColor(255, 255, 100);
        ofDrawBitmapString("Threshold: "+ofToString(threshold), ofPoint(10,40));
    }
    else {
#ifdef BLACKMAGIC
        
        cam.getGrayTexture().draw(0, 0, ofGetWidth(), ofGetHeight());
        ;
#else
        img.draw(0, 0, ofGetWidth(), ofGetHeight());
#endif
    }
    ofPushMatrix();
    ofScale(ofGetWidth(), ofGetHeight());
    
    //         ofxCv::drawMat(cvBwImage, 0, 0,1,1);
    
    
    for(int u=0;u<trackers.size();u++){
        ofPushMatrix();
        ofTranslate(trackers[u].roiRect.x/1920., trackers[u].roiRect.y/1080.);
        
        
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
        
        
        ofSetColor(255, 255, 0);
        ofNoFill();
        ofRect(0, 0, trackers[u].roiRect.width/1920., trackers[u].roiRect.height/1080.);
        ofFill();
        ofPopMatrix();
    }
    
    int s = blobs.size();
    for (int i=0; i<s; i++){
        float X=blobs[i].pt.x/(1920.);
        float Y=blobs[i].pt.y/(1080.);
        
        ofSetColor(255, 255, 0);
        
        
        ofCircle(X, Y, 0.003);
    }
    
    
    
    ofPopMatrix();
    
    
    
    
    ofSetColor(255,255,255);
    
    ofDrawBitmapString(ofToString(ofGetFrameRate()), ofPoint(10,20));
    
    /*calibration.getDistortedIntrinsics().loadProjectionMatrix();
     applyMatrix(modelMatrix);
     ofMesh mesh;
     mesh.setMode(OF_PRIMITIVE_POINTS);
     for(int i = 0; i < objectPoints.size(); i++) {
     mesh.addVertex(toOf(objectPoints[i]));
     }
     glPointSize(3);
     ofSetColor(magentaPrint);
     mesh.drawVertices();
     
     
     glEnable(GL_DEPTH_TEST);
     glScaled(1, 1, -1);
     ofDrawAxis(10);
     
     glTranslated(5*0.65, 4.5, 0);
     ofDrawAxis(10);
     glDisable(GL_DEPTH_TEST);*/
    
}

void testApp::keyPressed(int key){
    if(key == 't'){
        setThreshold = !setThreshold;
    }
    if(key=='f'){
        ofToggleFullscreen();
    }
}

void testApp::keyReleased(int key){
    /*    if(key == 't'){
     setThreshold = false;
     }*/
    
}

void testApp::mouseMoved(int x, int y){
    lastMouse = ofVec3f(x,y,0);
}

void testApp::mouseDragged(int x, int y, int button){
    if(setThreshold){
        threshold = x-lastMouse.x;
        settings.setValue("threshold", threshold);
        settings.save("settings.xml");
    }
}
