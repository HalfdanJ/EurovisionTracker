#include "testApp.h"

int patternWidth = 5;
int patternHeight = 3;


void testApp::setup() {
    
    oscReceiver.setup(8080);
    
    //ofSetFrameRate(50);
    //ofSetVerticalSync(true);
    TIME_SAMPLE_SET_FRAMERATE(25.f);
    TIME_SAMPLE_SET_DRAW_LOCATION( TIME_MEASUREMENTS_TOP_RIGHT );
    
    
    //Settings
    OFX_REMOTEUI_SERVER_SETUP(10000); //start server
    OFX_REMOTEUI_SERVER_SHARE_PARAM(debug,0,1);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(threshold,0,255);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(blobMinSize,10,600);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(blobMaxSize,0,6000);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(roiSize,0,1000);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(numTrackers,0,3);
    OFX_REMOTEUI_SERVER_SHARE_PARAM(scale,0.1,0.5);
    OFX_REMOTEUI_SERVER_LOAD_FROM_XML();



    //Allocation
    for(int i=0;i<3;i++){
        mask[i].allocate(1920, 1080);
        boxContent[i].allocate(1920, 1080);
        composed[i].allocate(1920, 1080);
    }
    output.allocate(1920, 1080);
    
    //Tracker setup, creates 3 trackers for reuse, since they are expensive to create
    for(int i=0;i<3;i++){
        Tracker tracker;
        tracker.patternDefinition.x = patternWidth;
        tracker.patternDefinition.y = patternHeight;
        unusedTrackers.push_back(tracker);
    }
    
    
    //Syphon
    syphon.setup();
    syphon.set("Syphon","QLab");
    
    syphonOut.setName("eurovision");
    //syphon.set("Screen 1","Millumin");
    
    //Scene setup
    ofSetSmoothLighting(true);
    
    pointLight.setDiffuseColor( ofFloatColor(.85, .85, .85) );
//    pointLight.setSpecularColor( ofFloatColor(1.f, 1.f, 1.f));
    pointLight.setPosition(1000, -1000, 4000);

    
  /*  pointLight2.setDiffuseColor( ofFloatColor( 238.f/255.f, 57.f/255.f, 135.f/255.f ));
    pointLight2.setSpecularColor(ofFloatColor(.8f, .8f, .9f));
    
    pointLight3.setDiffuseColor( ofFloatColor(19.f/255.f,94.f/255.f,77.f/255.f) );
    pointLight3.setSpecularColor( ofFloatColor(18.f/255.f,150.f/255.f,135.f/255.f) );
    */
    // shininess is a value between 0 - 128, 128 being the most shiny //
	material.setShininess( 120 );
    // the light highlight of the material //
	material.setSpecularColor(ofColor(255, 255, 255, 255));
    material.setAmbientColor(ofFloatColor(0.2,0.2,0.2));
    
    content.setPixelFormat(OF_PIXELS_RGBA);
    content.loadMovie("ESC_Content_start.mov");
  //  content.play();
    content.setVolume(0.1);
    
    textureFront[0].loadImage("textureFront.png");
    textureFront[1].loadImage("textureFront2.png");
    textureFront[2].loadImage("textureFront3.png");
#ifdef SYPHON
    syphonCam.setup();
    syphonCam.set("Screen 1","Millumin");
    img.allocate(1920, 1080, OF_IMAGE_COLOR);

#endif
    
    
#ifdef SIMULATOR
    //Simulator setup
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
    //Blackmagic setup
    cam.setup(1920, 1080, 30);
    
#endif
    
    
#ifdef VIDEO
    //Video setup
    img.loadMovie("3boxes_threshold_100.mov");
    //img.play();
#endif
    
  //  ofSetWindowPosition(-1900, 0);
    //ofToggleFullscreen();
    
    
    textureBack.loadImage("Box_Back.png");
    textureSide.loadImage("Box_Side.png");
    textureTop.loadImage("Box_Side.png");
    textureGradient.loadImage("Tex_Grad.png");
    
}


//---------------------------------------------------------------------------------------------------------

int frame;
void testApp::update() {
   frame = ofGetFrameNum()  +2600;
   frame = ofGetMouseX()+1200;
    content.setFrame( (float)frame  );
    img.setFrame( (float)(frame)  );
    
    content.update();
    
    while(oscReceiver.hasWaitingMessages()){
        ofxOscMessage m;
        oscReceiver.getNextMessage(&m);
        
        if(m.getAddress() == "/trackers/num"){
            numTrackers = m.getArgAsInt32(0);
        }
    }
    
    
    bool update = true;
    
    updateSimulator();
    
    TIME_SAMPLE_SET_ENABLED(debug);

    
#ifdef VIDEO
    img.update();
#endif
    
#ifdef BLACKMAGIC
    update = cam.update();
#endif
    
    //Update the tracker (if there is a new frame)
    if(update){
        updateTracker();
    }
}

//---------------------------------------------------------------------------------------------------------

void testApp::updateSimulator(){
#ifdef SIMULATOR
    TIME_SAMPLE_START("Simulator");


    
    //Draw the simulator fbo image
    simulatorFbo.begin();{
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
    }simulatorFbo.end();
    
    //Put the fbo into the ofImage img (expensive shit)
    ofPixels pixels;
    simulatorFbo.readToPixels(pixels);
    img.setFromPixels(pixels);
    
    TIME_SAMPLE_STOP("Simulator");

#endif

}


//---------------------------------------------------------------------------------------------------------

void testApp::updateTracker(){
    TIME_SAMPLE_START("ImagePreproc");

#ifdef BLACKMAGIC
    cvBwImage = ofxCv::toCv(cam.getGrayPixels());
#elif SYPHON
    ofPixels pixels;
    syphonCam.mTex.readToPixels(pixels);
    img.setFromPixels(pixels);
#else
    cv::Mat cvImage = ofxCv::toCv(img);
    cv::cvtColor(cvImage, cvBwImage, CV_RGB2GRAY);
#endif
    TIME_SAMPLE_STOP("ImagePreproc");
    
    TIME_SAMPLE_START("Tracker");

    dispatch_queue_t trackerQueue = dispatch_queue_create("com.halfdanj.tracker", 0);
    
    //Run a debug tracker (multithreaded with the other trackers)
    if(debug){
        dispatch_async(trackerQueue, ^{
            debugTracker.lowThreshold = threshold;
            debugTracker.highThreshold = threshold+10;
            debugTracker.blobMaxSize  = blobMaxSize;
            debugTracker.blobMinSize = blobMinSize;
            debugTracker.roiSize = roiSize;

            blobs =debugTracker.debugTrack(cvBwImage);
        });
    }
    
    
    bool trackerFound[3] = {false,false,false}, *trackerFoundPtr;
    trackerFoundPtr = trackerFound;
    
    int num = MIN(numTrackers, trackers.size());
    
    //Run the exsisting trackers (multrithreaded)
    for(int i=0;i<num;i++){
        trackers[i].lowThreshold = threshold;
        trackers[i].highThreshold = threshold+10;
        trackers[i].blobMaxSize  = blobMaxSize;
        trackers[i].blobMinSize = blobMinSize;
        trackers[i].roiSize = roiSize;
        
        //dispatch_async(trackerQueue, ^{
            bool found = trackers[i].update(cvBwImage);
            if(found){
                trackerFoundPtr[i] = true;
            }
        //});
        
    }
    
    //Wait for the trackers to complete
    dispatch_sync(trackerQueue, ^{});
    
    //Look for trackers that are the same
    for(int i=0;i<num;i++){
        for(int u=i+1;u<trackers.size();u++){
            ofPoint roiTl1 = ofxCv::toOf(trackers[i].roiRect.tl());
            ofPoint p1 = ofxCv::toOf(trackers[i].imagePoints[0]) + roiTl1;
            
            ofPoint roiTl2 = ofxCv::toOf(trackers[u].roiRect.tl());
            ofPoint p2 = ofxCv::toOf(trackers[u].imagePoints[0]) + roiTl2;
            
            if(p1.distance(p2) < 2 ){
                trackerFound[u] = false;
            }
        }
    }
    
    for(int i=num;i<trackers.size();i++){
        trackerFound[i] = false;
    }
    
    //Delete trackers that have disappeared
    for(int i=0;i<trackers.size();i++){
        if(!trackerFoundPtr[i]){
            cout<<"Delete "<<i<<endl;
            unusedTrackers.push_back(trackers[i]);
            trackers.erase(trackers.begin()+i);
            
        }
    }
    
    TIME_SAMPLE_STOP("Tracker");
    TIME_SAMPLE_START("NewTracker");
    
    
    //Add new trackers if there are missing some
    if(trackers.size() < numTrackers){

        //Draw a rectangle on top of all the other trackers so they dont get tracked
        for(int i=0;i<trackers.size();i++){
            cv::Point cornerPoints[4];
            int s = trackers[i].imagePoints.size();
            if(s>0){
                cornerPoints[3] = trackers[i].imagePoints[0];
                cornerPoints[1] = trackers[i].imagePoints[s-1];
                cornerPoints[2] = trackers[i].imagePoints[trackers[i].patternDefinition.y-1];
                cornerPoints[0] = trackers[i].imagePoints[s-trackers[i].patternDefinition.y];
                
                const cv::Point* ppt[1] = { cornerPoints };
                int npt[] = { 4 };
                cv::fillPoly( cvBwImage,
                             ppt,
                             npt,
                             1,
                             cv::Scalar( 255, 255, 255 ),
                             8 );
            }
        }
        
        //Create the new tracker
        Tracker newTracker = unusedTrackers[unusedTrackers.size()-1];
        newTracker.lowThreshold = threshold;
        newTracker.highThreshold = threshold+10;
        newTracker.blobMaxRoundiness = blobMaxRoundiness;
        newTracker.blobMinRoundiness = blobMinRoundiness;
        newTracker.blobMaxSize  = blobMaxSize;
        newTracker.blobMinSize = blobMinSize;
        newTracker.roiSize = roiSize;
        
        
        newTracker.lastLocation = cv::Point();
        bool found = newTracker.update(cvBwImage);
        if(found){
            unusedTrackers.pop_back();
            trackers.push_back(newTracker);
        }
    }
    TIME_SAMPLE_STOP("NewTracker");
    
    
    //Sort the trackers after x position
    std::sort(trackers.begin(), trackers.end());
    
    
    
    
    
    
}
//---------------------------------------------------------------------------------------------------------

void testApp::draw() {
    output.begin();
    ofClear(0, 0, 0);
    ofSetColor(255,255,255);
    ofDrawBitmapString("Frame: "+ofToString(frame ), ofPoint(10,20));
    
    ofDrawBitmapString("Frame: "+ofToString(content.getPosition() * content.getDuration() *25.0  ), ofPoint(10,40));
    ofDrawBitmapString("Frame: "+ofToString(img.getPosition() * img.getDuration() *25.0  ), ofPoint(10,60));

    if(setThreshold){
        cv::threshold(cvBwImage, cvBwImage,threshold, 255, cv::THRESH_BINARY);
        ofxCv::drawMat(cvBwImage,0,0,ofGetWidth(),ofGetHeight());
        
        ofSetColor(255, 255, 100);
        ofDrawBitmapString("Threshold: "+ofToString(threshold), ofPoint(10,40));
    }
    else {
#ifdef BLACKMAGIC
     
        // cam.getColorTexture().draw(0, 0, ofGetWidth(), ofGetHeight());
#else
     if(debug)
         img.draw(0, 0, ofGetWidth(), ofGetHeight());
#endif
    }

    
    
    //Debug stuff
    if(debug){
        ofPushMatrix();
        ofScale(ofGetWidth(), ofGetHeight());

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
         
            ofScale(1.0/ofGetWidth(), 1.0/ofGetHeight());
            ofSetColor(255,255,0);
            ofDrawBitmapString(ofToString(u), trackers[u].roiRect.x+10,trackers[u].roiRect.y+20);
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
    
    }
    
    
    ofSetColor(255,255,255);

    if(debug){
        ofDrawBitmapString(ofToString(ofGetFrameRate())+" "+ofToString(ofGetWidth())+"x"+ofToString(ofGetHeight()), ofPoint(10,20));
    }
    

    if(!debug){
        for(int i=0;i<trackers.size();i++){
            drawBox(i,0);
        }
    }
    output.end();
    
    
    
    if(img.isFrameNew())
        syphonOut.publishTexture(&output.getTextureReference());
    
    output.draw(0,0);
    
    ofPixels pixels;
  //  output.readToPixels(pixels);
   // ofSaveImage(pixels, "frames/frame"+ofToString(frame)+".png");
    
    
}
//---------------------------------------------------------------------------------------------------------

void testApp::drawBox(int box, int mode){
    
    float patternAspect = 1.3;
    ofVec2f offset;
 /*   if(box == 0){
        offset = ofVec2f(-0.7,0);
    }*/
    if(box == 1){
        offset = ofVec2f(-1,-0);
    }
    if(box == 2){
        offset = ofVec2f(-2.1,-0.8);
    }
    
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
        
        ofTranslate(offset);
        
        ofSetColor(255, 255, 255);
        ofRect(-25, -25, 0, 50, 50);
        
        glPopMatrix();
        
        ofDisableDepthTest();
    } mask[box].end();
    
    
    /*trackers[box].getDistortedIntrinsics().loadProjectionMatrix();
     ofxCv::applyMatrix(trackers[box].modelMatrix);
     
     ofSetColor(255,0,0);
     ofRect(0, 0, 10*1.3, 10);*/
    
    boxContent[box].begin();{
        ofClear(0, 0, 0);
        ofSetHexColor(0x666666);
        
        
        ofEnableAlphaBlending();
        
        ofEnableDepthTest();
        
     //   ofEnableLighting();
      //  pointLight.enable();
/*        pointLight2.enable();
        pointLight3.enable();*/
        
        material.begin();
        
        
        
        
        // Set the matrix to the perspective of this marker
        // The origin is in the middle of the marker
        ofPushMatrix();{
            
            //Apply 3d
            trackers[box].getDistortedIntrinsics().loadProjectionMatrix();
            ofxCv::applyMatrix(trackers[box].modelMatrix);
            
            ofEnableSmoothing();
            
            ofTranslate(5*patternAspect, 5);
            glScaled(scale, scale, -scale);
            ofTranslate(offset);
            
            ofFill();
            ofSetColor(255,255,255,255);
            
            //backwall
            textureBack.draw(-25, -25, -50, 50, 50);
            
            
            //top/bottom wall
            ofPushMatrix();{
                ofRotate(90, 0, 1, 0);
                textureTop.draw(-0, -25, -25, 50, 50);
                textureTop.draw(-0, -25, 25, 50, 50);
            } ofPopMatrix();
           
            //Sides wall
            ofPushMatrix();{
                ofRotate(90, 0, 1, 0);
                ofRotate(90, 1, 0, 0);
                textureSide.draw(-0, -25, -25, 50, 50);
                textureSide.draw(-0, -25, 25, 50, 50);
            }ofPopMatrix();
            
      //      textureGradient.draw(-25, -25, 0, 50, 2);
    //        textureGradient.draw(-25, 25, 0, 50, -2);
            
           // textureFront[box].draw(-25,-25,50,50);
            ofPushMatrix();{
                ofRotate(90, 0, 0, 1);
                //                textureGradient.draw(-25, -25, 0, 50, 2);
                //              textureGradient.draw(-25, 25, 0, 50, -2);
                
            } ofPopMatrix();
            
            ofPushMatrix();{
                //img.draw(-15,-2, 30, 30);
                ofFill();
                ofSetColor(255,255,255);
//                ofTranslate(0, 0,-25);
                // syphon.draw(-25, -25, 50,50);
                /*
                 FEATURE: Adding Syphon-input to the box
                 */
                content.getTextureReference().bind();
                
                int b = box +1;
                float w = content.getWidth()/3.0;
              /*glBegin(GL_QUADS);
                glTexCoord2d(w*b+w, 0); glVertex3d(-25, 25, -25);
                glTexCoord2d(w*b, 0); glVertex3d(25, 25, -25);
                glTexCoord2d(w*b, content.getHeight()); glVertex3d(25, -25, -25);
                glTexCoord2d(w*b+w, content.getHeight()); glVertex3d(-25, -25, -25);
                glEnd();
            */

             //   ofDisableDepthTest();
               /* ofSetColor(0, 0, 0);
                glBegin(GL_QUADS);
                glTexCoord2d(w*box+w, 0); glVertex3d(-25, -25, -75);
                glTexCoord2d(w*box, 0); glVertex3d(25, -25, -75);
                glTexCoord2d(w*box, content.getHeight()); glVertex3d(25, -25, -25);
                glTexCoord2d(w*box+w, content.getHeight()); glVertex3d(-25, -25, -25);
                glEnd();
                
                glBegin(GL_QUADS);
                glTexCoord2d(w*box+w, 0); glVertex3d(-25, 0, -50);
                glTexCoord2d(w*box, 0); glVertex3d(25, 0, -50);
                glTexCoord2d(w*box, content.getHeight()); glVertex3d(25, -50, -50);
                glTexCoord2d(w*box+w, content.getHeight()); glVertex3d(-25, -50, -50);

                glEnd();*/

                ofSetColor(255, 255, 255);
                
                
                
                content.getTextureReference().unbind();
                
            }ofPopMatrix();
            
            //   ofDisableLighting();
            ofDisableDepthTest();
            
        } ofPopMatrix();
        
        // material.end();
        
        
    } boxContent[box].end();
    
    ofFill();
    
    
    ofSetColor(255,255,255,255);
    
    glEnable(GL_BLEND);
    
    composed[box].begin();{
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
        
    }composed[box].end();
    
    
    ofEnableAlphaBlending();
    
    
    //camera.draw(0,0,1024,680);
    
    
    
    ofPushMatrix();
    glScaled(1, -1, 1);
    glTranslated(0,-ofGetHeight(), 0);
    
    composed[box].draw(0, 0);
    ofPopMatrix();
    
    

}
//---------------------------------------------------------------------------------------------------------

void testApp::keyPressed(int key){
    if(key == 't'){
        setThreshold = !setThreshold;
    }
    if(key=='f'){
        ofToggleFullscreen();
    }

    if(key=='d'){
        debug = !debug;
    }
}
//---------------------------------------------------------------------------------------------------------

void testApp::keyReleased(int key){
    /*    if(key == 't'){
     setThreshold = false;
     }*/
    
}
//---------------------------------------------------------------------------------------------------------

void testApp::mouseMoved(int x, int y){
    lastMouse = ofVec3f(x,y,0);
}
//---------------------------------------------------------------------------------------------------------

void testApp::mouseDragged(int x, int y, int button){
    if(setThreshold){
        threshold = x-lastMouse.x;
    }
}
