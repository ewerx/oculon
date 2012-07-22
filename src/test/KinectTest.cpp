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
#include "KinectController.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include "cinder/CinderMath.h"
#include <boost/format.hpp>


using namespace ci;

KinectTest::KinectTest()
: Scene("kinect")
{
}

KinectTest::~KinectTest()
{
}

void KinectTest::setup()
{
    Scene::setup();
    
    mBlobTracker.setup( mApp->getKinectController() );
    mKinectTilt = 0.0f;
}

void KinectTest::update(double dt)
{
    mBlobTracker.update();
    
    Scene::update(dt);
}

void KinectTest::draw()
{
    gl::pushMatrices();
    gl::setMatricesWindowPersp(getWindowSize());

    const int centerX = getWindowWidth() / 2;
    
    KinectController& kinectCtrl = mApp->getKinectController();
    
    gl::Texture depthTexture = kinectCtrl.getDepthTexture();
    gl::Texture colorTexture = kinectCtrl.getVideoTexture();
    gl::Texture cvTexture = mBlobTracker.getCvTexture();
    Surface depthSurface = kinectCtrl.getDepthSurface();
    
    if( depthTexture )
    {
        gl::draw( depthTexture, Rectf( centerX, 0, 0, centerX/depthTexture.getAspectRatio() ) );
    }
		
	if( colorTexture )
    {
		gl::draw( colorTexture, Rectf( getWindowWidth(), 0, centerX, centerX/colorTexture.getAspectRatio() ) );
    }
    
    if( cvTexture )
    {
        gl::draw( cvTexture, Rectf( centerX, getWindowHeight()/2, 0, getWindowHeight() ) );
    }
    
    if( depthSurface )
    {
        gl::draw( depthSurface, Rectf( getWindowWidth(), getWindowHeight()/2, centerX, getWindowHeight() ) );
    }
    
    gl::color(Colorf(1.0f, 0.0f, 0.0f));
    gl::drawSphere(mBlobTracker.getTargetPosition(), 10.0f);
    
    gl::popMatrices();
}

void KinectTest::drawDebug()
{
    char buf[256];
    //const Vec3f& pos = mBlobTracker.getTargetPosition();
    //snprintf(buf, 256, "tracking: %.1f,%.1f,%.1f", pos.x, pos.y, pos.z );
    //mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.7f, 0.8f));
    
    Scene::drawDebug();
}

bool KinectTest::handleKeyDown(const KeyEvent& keyEvent)
{
    const float TILT_INCREMENT = 5.0f;
    const float TILT_MAX = 90.f;
    const float TILT_MIN = -90.0f;
    
    bool handled = true;
    
    Kinect& kinect = mApp->getKinectController().getKinect();
    
    switch (keyEvent.getCode()) 
    {
        case KeyEvent::KEY_UP:
            mKinectTilt = min( (mKinectTilt + TILT_INCREMENT), TILT_MAX );
            kinect.setTilt(mKinectTilt);
            break;
        case KeyEvent::KEY_DOWN:
            mKinectTilt = max( (mKinectTilt - TILT_INCREMENT), TILT_MIN );
            kinect.setTilt(mKinectTilt);
            break;
        case KeyEvent::KEY_i:
            kinect.setVideoInfrared(!kinect.isVideoInfrared());
            break;
        default:
            handled = false;
            break;
    }
    
    return handled;    
}
