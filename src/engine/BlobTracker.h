/*
 *  BlobTracker.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __BLOBTRACKER_H__
#define __BLOBTRACKER_H__

#include "cinder/Vector.h"
#include "cinder/gl/Texture.h"
#include "KinectController.h"


class BlobTracker {
public:
	BlobTracker();
    virtual ~BlobTracker();
    
    void setup(KinectController& kinect);
	void update();
	void draw();
    
    ci::Vec3f getTargetPosition() const        { return mTargetPosition; }
    ci::gl::Texture getCvTexture() const       { return mCvTexture; }

private:
    KinectController* mKinectController;
    
    float mThreshold;
    float mBlobMin;
    float mBlobMax;
    float mKinectTilt;
    
    ci::gl::Texture mCvTexture;
    
    ci::Vec3f mTargetPosition;
    
};

#endif // __BlobTracker_H__
