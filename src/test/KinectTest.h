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
#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "Kinect.h"
#include "CinderOpenCV.h"


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
    
    Kinect              mKinect;
    float               mKinectTilt;
    bool                mIsVideoInfrared;
    
    gl::Texture         mColorTexture;
    gl::Texture         mDepthTexture;
    
    Surface             mColorSurface;
    cv::Mat             mPrevFrame;

    // hand tracking
    vector<cv::Point2f> mPrevFeatures;
    vector<cv::Point2f> mFeatures;
    vector<uint8_t>     mFeatureStatuses;  
    
};

#endif // __KINECTEST_H__
