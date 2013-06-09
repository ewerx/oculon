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
#include "CinderFreenect.h"

using namespace std;

class KinectController 
{
public:
	KinectController();
    virtual ~KinectController();
    
    void setup();
	void update();
    
    ci::gl::Texture getVideoTexture() const { return mVideoTexture; }
    ci::gl::Texture getDepthTexture() const { return mDepthTexture; }
    ci::Surface getDepthSurface() const     { return mDepthSurface; }
    bool hasNewDepthFrame() const           { return mHasNewDepthFrame; }
    bool hasNewVideoFrame() const           { return mHasNewVideoFrame; }
    
    ci::Kinect& getKinect()                 { return mKinect; }

private:
    ci::Kinect              mKinect;
    
    ci::gl::Texture         mVideoTexture;
    ci::gl::Texture         mDepthTexture;
    ci::Surface             mDepthSurface;
    
    bool                    mHasNewDepthFrame;
    bool                    mHasNewVideoFrame;
    
    int                     mNumKinects;
    
    //ci::Surface               mColorSurface;
    
};

#endif // __KinectController_H__
