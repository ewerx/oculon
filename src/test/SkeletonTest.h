/*
 *  SkeletonTest.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __SKELETONTEST_H__
#define __SKELETONTEST_H__

#include "Scene.h"
#include "_2RealKinect.h"

using namespace _2Real;

//
// Audio input tests
//
class SkeletonTest : public Scene
{
public:
    SkeletonTest();
    virtual ~SkeletonTest();
    
    // inherited from Scene
    void setup();
    //void setupParams(params::InterfaceGl& params);
    //void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    bool handleKeyDown(const KeyEvent& keyEvent);

    void handleHandBegan(HandEvent event);
    void handleHandMoved(HandEvent event);
    void handleHandEnded(HandEvent event);
    void handleGestureRecognized(GestureEvent event);
    void handleGestureProcessed(GestureEvent event);
    
private:
    
    _2RealKinect*				m_2RealKinect;
    bool						m_bIsMirroring;
    int							m_iKinectWidth;
    int							m_iKinectHeight;
    int							m_iScreenWidth;
    int							m_iScreenHeight;
    int							m_iNumberOfDevices;
    ci::Vec2f					m_ImageSize;
    ci::Font					m_Font;
    int							m_iMotorValue;
    
    Vec3f               mHandPos;
    
    //Surface             mColorSurface;
    

    // hand tracking -- where is this from???
    //cv::Mat             mPrevFrame;
    //vector<cv::Point2f> mPrevFeatures;
    //vector<cv::Point2f> mFeatures;
    //vector<uint8_t>     mFeatureStatuses;  
    
};

#endif // __KINECTEST_H__
