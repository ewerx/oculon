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
        ImageSourceRef depthImage = mKinect.getDepthImage();
        if( depthImage )
        {
            mHasNewDepthFrame = true;
            mDepthTexture = depthImage;
            mDepthSurface = depthImage;
        }
    }
	
	if( mKinect.checkNewVideoFrame() )
    {
        mHasNewVideoFrame = true;
		mVideoTexture = mKinect.getVideoImage();
    }
}


