/*
 *  KinectController.h
 *  Oculon
 *
 *  Created by Ehsan on 22-Mar-2012.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __KINECTCONTROLLER_H__
#define __KINECTCONTROLLER_H__


#include "cinder/gl/Texture.h"
#include "cinder/Surface.h"
#include "Kinect.h"

using namespace std;
using namespace ci;

class KinectController 
{
public:
	KinectController();
    virtual ~KinectController();
    
    void setup();
	void update();
    
    gl::Texture getVideoTexture() const { return mVideoTexture; }
    gl::Texture getDepthTexture() const { return mDepthTexture; }
    Surface getDepthSurface() const     { return mDepthSurface; }
    bool hasNewDepthFrame() const       { return mHasNewDepthFrame; }
    bool hasNewVideoFrame() const       { return mHasNewVideoFrame; }
    
    Kinect& getKinect()                 { return mKinect; }

private:
    Kinect              mKinect;
    
    gl::Texture         mVideoTexture;
    gl::Texture         mDepthTexture;
    Surface             mDepthSurface;
    
    bool                mHasNewDepthFrame;
    bool                mHasNewVideoFrame;
    
    //Surface             mColorSurface;
    
};

#endif // __KinectController_H__
