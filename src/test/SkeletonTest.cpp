/*
 *  SkeletonTest.cpp
 *  Oculon
 *
 *  Created by Ehsan on 12-03-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "OculonApp.h"
#include "SkeletonTest.h"
#include "Resources.h"
#include "OpenNI.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include "cinder/CinderMath.h"
#include <boost/format.hpp>


using namespace ci;

SkeletonTest::SkeletonTest()
{
}

SkeletonTest::~SkeletonTest()
{
    m_2RealKinect->shutdown();
}

void SkeletonTest::setup()
{
    mOpenNI.registerHandBegan(this, &SkeletonTest::handleHandBegan);
    mOpenNI.registerHandMoved(this, &SkeletonTest::handleHandMoved);
    mOpenNI.registerHandEnded(this, &SkeletonTest::handleHandEnded);
    
    mOpenNI.registerGestureRecognized(this, &SkeletonTest::handleGestureRecognized);
    mOpenNI.registerGestureProcessed(this, &SkeletonTest::handleGestureProcessed);
    
    mOpenNI.addGesture("Wave");
    
    mOpenNI.start();
    mOpenNI.startHandsTracking(Vec3f::zero());
}

void SkeletonTest::update(double dt)
{
}

void SkeletonTest::draw()
{
    gl::pushMatrices();
    gl::setMatricesWindowPersp(getWindowSize());

    const int centerX = getWindowWidth() / 2;
    
        
    gl::color(Colorf(1.0f, 0.0f, 0.0f));
    //gl::drawSphere(mBlobTracker.getTargetPosition(), 10.0f);
    
    gl::popMatrices();
}

void SkeletonTest::drawDebug()
{
    char buf[256];
    const Vec3f& pos = Vec3f::zero();
    snprintf(buf, 256, "tracking: %.1f,%.1f,%.1f", pos.x, pos.y, pos.z );
    mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.7f, 0.8f));
}

bool SkeletonTest::handleKeyDown(const KeyEvent& keyEvent)
{    
    bool handled = true;
    
    switch (keyEvent.getCode()) 
    {
        default:
            handled = false;
            break;
    }
    
    return handled;    
}

void SkeletonTest::handleHandBegan(HandEvent event)
{
    console() << "[hand] began: " << event.getPos() << std::endl;
}

void SkeletonTest::handleHandMoved(HandEvent event)
{
    console() << "[hand] moved: " << event.getPos() << std::endl;
}

void SkeletonTest::handleHandEnded(HandEvent event)
{
    console() << "[hand] ended: " << event.getPos() << std::endl;
    
    mOpenNI.startHandsTracking(Vec3f::zero());
}


void SkeletonTest::handleGestureRecognized(GestureEvent event)
{
    console() << "[hand] gesture recognized: " << event.mGesture << std::endl;
}

void SkeletonTest::handleGestureProcessed(GestureEvent event)
{
    console() << "[hand] gesture processed: " << event.mGesture << std::endl;
}
