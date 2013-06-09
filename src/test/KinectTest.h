/*
 *  KinectTest.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __KINECTEST_H__
#define __KINECTEST_H__

#include "Scene.h"
//#include "BlobTracker.h"

//
// Audio input tests
//
class KinectTest : public Scene
{
public:
    KinectTest();
    virtual ~KinectTest();
    
    // inherited from Scene
    void setup();
    //void setupParams(params::InterfaceGl& params);
    //void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    bool handleKeyDown(const KeyEvent& keyEvent);

    
private:
    
    float               mKinectTilt;
    
//    BlobTracker         mBlobTracker;
    
    //Surface             mColorSurface;
    

    // hand tracking -- where is this from???
    //cv::Mat             mPrevFrame;
    //vector<cv::Point2f> mPrevFeatures;
    //vector<cv::Point2f> mFeatures;
    //vector<uint8_t>     mFeatureStatuses;  
    
};

#endif // __KINECTEST_H__
