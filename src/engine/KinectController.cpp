/*
 *  KinectController.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "KinectController.h"
#include "cinder/gl/Texture.h"
#include "cinder/app/AppBasic.h"

using namespace ci;
using namespace ci::app;

KinectController::KinectController()
: mDepthTexture()
, mColorTexture()
{
}

KinectController::~KinectController()
{
}

void KinectController::setup()
{
    console() << "[kinect] " << Kinect::getNumDevices() << " Kinects connected." << std::endl;
    mKinect = Kinect( Kinect::Device() ); // the default Device implies the first Kinect connected
}


void KinectController::update()
{
    mHasNewVideoFrame = false;
    mHasNewDepthFrame = false;
    //TODO: multithreaded
    if( mKinect.checkNewDepthFrame() )
    {
        mHasNewDepthFrame = true;
		mDepthTexture = mKinect.getDepthImage();
    }
	
	if( mKinect.checkNewVideoFrame() )
    {
        mHasNewVideoFrame = true;
		mColorTexture = mKinect.getVideoImage();
    }
}


