/*
 *  KinectTest.cpp
 *  Oculon
 *
 *  Created by Ehsan on 12-03-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "OculonApp.h"
#include "KinectTest.h"
#include "Resources.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include "cinder/CinderMath.h"
#include <boost/format.hpp>


using namespace ci;

KinectTest::KinectTest()
{
}

KinectTest::~KinectTest()
{
}

void KinectTest::setup()
{
    console() << "[kinect] " << Kinect::getNumDevices() << " Kinects connected." << std::endl;
    mKinect = Kinect( Kinect::Device() ); // the default Device implies the first Kinect connected
    mKinectTilt = 0.0f;
    mIsVideoInfrared = false;
}

void KinectTest::update(double dt)
{
    if( mKinect.checkNewDepthFrame() )
		mDepthTexture = mKinect.getDepthImage();
	
	if( mKinect.checkNewVideoFrame() )
		mColorTexture = mKinect.getVideoImage();
}

void KinectTest::draw()
{
    gl::pushMatrices();
    gl::setMatricesWindow(getWindowSize());

    const int centerX = getWindowWidth() / 2;
    
    if( mDepthTexture )
    {
        gl::draw( mDepthTexture, Rectf( centerX, 0, 0, centerX/mDepthTexture.getAspectRatio() ) );
    }
		
	if( mColorTexture )
    {
		gl::draw( mColorTexture, Rectf( getWindowWidth(), 0, centerX, centerX/mColorTexture.getAspectRatio() ) );
    }
    
    gl::popMatrices();
}

void KinectTest::drawDebug()
{
    //char buf[256];
    //snprintf(buf, 256, "kinect accel: %.1f,%.1f,%.1f", mKinect.getAccel().x, mKinect.getAccel().y, mKinect.getAccel().z );
    //mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.7f, 0.8f));
}

bool KinectTest::handleKeyDown(const KeyEvent& keyEvent)
{
    const float TILT_INCREMENT = 5.0f;
    const float TILT_MAX = 90.f;
    const float TILT_MIN = -90.0f;
    
    bool handled = true;
    
    switch (keyEvent.getCode()) 
    {
        case KeyEvent::KEY_UP:
            mKinectTilt = min( (mKinectTilt + TILT_INCREMENT), TILT_MAX );
            mKinect.setTilt(mKinectTilt);
            break;
        case KeyEvent::KEY_DOWN:
            mKinectTilt = max( (mKinectTilt - TILT_INCREMENT), TILT_MIN );
            mKinect.setTilt(mKinectTilt);
            break;
        case KeyEvent::KEY_i:
            mIsVideoInfrared = !mIsVideoInfrared;
            mKinect.setVideoInfrared(mIsVideoInfrared);
            break;
        default:
            handled = false;
            break;
    }
    
    return handled;    
}
