#include "testApp.h"

int patternWidth = 5;
int patternHeight = 3;


void testApp::setup() {
    
    
    
    
    
    oscReceiver.setup(8080);
    
    ofSetFrameRate(50);
    ofSetVerticalSync(true);
    
    trackerReady = true;
    
    settings.loadFile("settings.xml");
    threshold = settings.getValue("threshold", 100);
    
    for(int i=0;i<3;i++){
        mask[i].allocate(1920, 1080);
        boxContent[i].allocate(1920, 1080);
        composed[i].allocate(1920, 1080);
    }
    
    ofSetSmoothLighting(true);
    pointLight.setDiffuseColor( ofFloatColor(.85, .85, .55) );
    pointLight.setSpecularColor( ofFloatColor(1.f, 1.f, 1.f));
    pointLight.setPosition(1000, 300, 0);

    pointLight2.setDiffuseColor( ofFloatColor( 238.f/255.f, 57.f/255.f, 135.f/255.f ));
    pointLight2.setSpecularColor(ofFloatColor(.8f, .8f, .9f));
    
    pointLight3.setDiffuseColor( ofFloatColor(19.f/255.f,94.f/255.f,77.f/255.f) );
    pointLight3.setSpecularColor( ofFloatColor(18.f/255.f,150.f/255.f,135.f/255.f) );
    
    // shininess is a value between 0 - 128, 128 being the most shiny //
	material.setShininess( 120 );
    // the light highlight of the material //
	material.setSpecularColor(ofColor(255, 255, 255, 255));

    
    
    
#ifdef SIMULATOR
    simulatorFbo.allocate(1920, 1080);
    
    simulatorPos[0].x = 100;
    simulatorPos[0].y = 200;
    
    simulatorPos[1].x = 400;
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
        
        
        ofTranslate(simulatorPos[i].x+mouseX, simulatorPos[i].y+250, -simulatorPos[i].z+300);
        
        ofScale(200, 200,200);
        ofRotate(mouseY, 0, 1, 0);
        ofRect(0, 0, 1, 1);
        
        float sizeX = 0.8;
        float sizeY = 2*sizeX * (1.0*patternHeight/patternWidth);
        float r = 0.05;
        
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
    
    dispatch_queue_t trackerQueue = dispatch_queue_create("com.halfdanj.tracker", 0);

    //Run a debug tracker (multithreaded with the other trackers)
    dispatch_async(trackerQueue, ^{
        debugTracker.lowThreshold = threshold;
        debugTracker.highThreshold = threshold+10;
        blobs =debugTracker.debugTrack(cvBwImage);
    });
    
    
    bool trackerFound[3] = {false,false,false}, *trackerFoundPtr;
    trackerFoundPtr = trackerFound;
    
    
    //Run the exsisting trackers (multrithreaded)
    for(int i=0;i<trackers.size();i++){
        dispatch_async(trackerQueue, ^{
            trackers[i].lowThreshold = threshold;
            trackers[i].highThreshold = threshold+10;
            
            bool found = trackers[i].update(cvBwImage);
            if(found){
                trackerFoundPtr[i] = true;
            }
        });
        
    }
    
    //Wait for the trackers to complete
    dispatch_sync(trackerQueue, ^{});
    
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
    
    if(trackers.size() < 3){
        
        cvBwImageClone = cvBwImage.clone();
        
        for(int i=0;i<trackers.size();i++){
            int lineType = 8;
            
            
            cv::Point rook_points[1][4];
            int s = trackers[i].imagePoints.size();
            if(s>0){
                rook_points[0][3] = cv::Point( trackers[i].imagePoints[0].x, trackers[i].imagePoints[0].y );
                rook_points[0][1] = cv::Point( trackers[i].imagePoints[s-1].x, trackers[i].imagePoints[s-1].y );
                rook_points[0][2] = cv::Point( trackers[i].imagePoints[trackers[i].patternDefinition.y-1].x, trackers[i].imagePoints[trackers[i].patternDefinition.y-1].y );
                
                rook_points[0][0] = cv::Point( trackers[i].imagePoints[s-trackers[i].patternDefinition.y].x, trackers[i].imagePoints[s-trackers[i].patternDefinition.y].y );
                
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
#else
        img.draw(0, 0, ofGetWidth(), ofGetHeight());
#endif
    }
    ofPushMatrix();
    ofScale(ofGetWidth(), ofGetHeight());
    
    //         ofxCv::drawMat(cvBwImage, 0, 0,1,1);
    
    
    for(int u=0;u<trackers.size();u++){
        ofPushMatrix();
        
        
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
        ofRect(trackers[u].roiRect.x/1920., trackers[u].roiRect.y/1080.,
               trackers[u].roiRect.width/1920., trackers[u].roiRect.height/1080.);
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
    
    ofDrawBitmapString(ofToString(ofGetFrameRate())+" "+ofToString(ofGetWidth())+"x"+ofToString(ofGetHeight()), ofPoint(10,20));
    
    for(int i=0;i<trackers.size();i++){
        drawBox(i);
    }
    
}

void testApp::drawBox(int box){
    
    float scale = 0.3;
    float patternAspect = 1.3;
    
    //First create the mask of the box
    mask[box].begin();{
        ofClear(0, 0, 0);
        glPushMatrix();
        
        //Apply 3d
        trackers[box].getDistortedIntrinsics().loadProjectionMatrix();
        ofxCv::applyMatrix(trackers[box].modelMatrix);
        
        ofEnableDepthTest();
        ofTranslate(5*patternAspect, 5);
        glScaled(scale, scale, scale);
        
        ofSetColor(255, 255, 255);
        ofRect(-25, -25, 0, 50, 50);
        
        glPopMatrix();
        
        ofDisableDepthTest();
    }mask[box].end();

    
    /*trackers[box].getDistortedIntrinsics().loadProjectionMatrix();
    ofxCv::applyMatrix(trackers[box].modelMatrix);

    ofSetColor(255,0,0);
    ofRect(0, 0, 10*1.3, 10);*/
    
    boxContent[box].begin();{
        ofClear(0, 0, 0);
        ofSetHexColor(0x666666);
    
      
        ofEnableAlphaBlending();
        
        ofEnableDepthTest();
        
        ofEnableLighting();
                pointLight.enable();
         pointLight2.enable();
         pointLight3.enable();
         
         material.begin();
        
        
        
        
        // Set the matrix to the perspective of this marker
        // The origin is in the middle of the marker
        ofPushMatrix();
        
        //Apply 3d
        trackers[box].getDistortedIntrinsics().loadProjectionMatrix();
        ofxCv::applyMatrix(trackers[box].modelMatrix);
        
        ofEnableSmoothing();

        ofTranslate(5*patternAspect, 5);
        glScaled(scale, scale, -scale);
        
        
        ofFill();
        ofSetColor(150,150,150,255);
        
        ofRect(-25, -25, -50, 50, 50);
        
        
        ofPushMatrix();
        ofRotate(90, 0, 1, 0);
        ofRect(-0, -25, -25, 50, 50);
        ofRect(-0, -25, 25, 50, 50);
        ofPopMatrix();
        
        ofPushMatrix();
        ofRotate(90, 0, 1, 0);
        ofRotate(90, 1, 0, 0);
        ofRect(-0, -25, -25, 50, 50);
        ofRect(-0, -25, 25, 50, 50);
        ofPopMatrix();
        
        ofPopMatrix();
        
        
        
        
        ofDisableDepthTest();
        ofNoFill();
        ofSetLineWidth(5);
        ofSetColor(255,255,255,255);
        
        ofRect(-25, -25, -50, 50, 50);
        
        
        ofPushMatrix();
        ofRotate(90, 0, 1, 0);
        ofRect(-0, -25, -25, 50, 50);
        ofRect(-0, -25, 25, 50, 50);
        ofPopMatrix();
        
        ofPushMatrix();
        ofRotate(90, 0, 1, 0);
        ofRotate(90, 1, 0, 0);
        ofRect(-0, -25, -25, 50, 50);
        ofRect(-0, -25, 25, 50, 50);
        ofPopMatrix();
        
        ofPopMatrix();
        
        ofPushMatrix();
        ofTranslate(0, 0,-25);
        img.draw(-15,-2, 30, 30);
        ofPopMatrix();
        
    
    
    
        // material.end();
        ofDisableLighting();
        ofDisableDepthTest();
    
    
    } boxContent[box].end();

    ofFill();

    
    ofSetColor(255,255,255,255);
    
    glEnable(GL_BLEND);
    
    composed[box].begin();
    ofClear(0,0,0);
    mask[box].getTextureReference().bind();
    glBegin(GL_QUADS);
    glTexCoord2d(0, 0); glVertex2d(0, 0);
    glTexCoord2d(ofGetWidth(), 0); glVertex2d(ofGetWidth(), 0);
    glTexCoord2d(ofGetWidth(), ofGetHeight()); glVertex2d(ofGetWidth(), ofGetHeight());
    glTexCoord2d(0, ofGetHeight()); glVertex2d(0, ofGetHeight());
    glEnd();
    mask[box].getTextureReference().unbind();
    
    glBlendFunc(GL_DST_COLOR, GL_ZERO);
    
    boxContent[box].getTextureReference().bind();
    glBegin(GL_QUADS);
    glTexCoord2d(0, 0); glVertex2d(0, 0);
    glTexCoord2d(ofGetWidth(), 0); glVertex2d(ofGetWidth(), 0);
    glTexCoord2d(ofGetWidth(), ofGetHeight()); glVertex2d(ofGetWidth(), ofGetHeight());
    glTexCoord2d(0, ofGetHeight()); glVertex2d(0, ofGetHeight());
    glEnd();
    boxContent[box].getTextureReference().unbind();
    
    composed[box].end();
    
    
    ofEnableAlphaBlending();
    
    
    //camera.draw(0,0,1024,680);
    
    
    ofPushMatrix();
    glScaled(1, -1, 1);
    glTranslated(0,-ofGetHeight(), 0);
    
    composed[box].draw(0, 0);
    ofPopMatrix();

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
