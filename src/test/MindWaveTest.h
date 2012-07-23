/*
 *  MindWaveTest.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __MINDWAVETEST_H__
#define __MINDWAVETEST_H__

#include "cinder/Cinder.h"
#include "cinder/audio/Input.h"
#include "Scene.h"

//
// Audio input tests
//
class MindWaveTest : public Scene
{
public:
    MindWaveTest() : Scene("mindwave") {}
    virtual ~MindWaveTest() {}
    
    // inherited from Scene
    void setup();
    //void reset();
    void update(double dt);
    void draw();
    
private:
    void drawGraphs();
    vector<float> mRawValues;
    vector<float> mMeditation;
    vector<float> mAttention;
    int mMaxValues;
    
    /*
    // Brainwaves
	ci::Path2d	mAlpha1;
    ci::Path2d	mAlpha2;
	ci::Path2d	mBeta1;
    ci::Path2d	mBeta2;
    ci::Path2d	mGamma1;
    ci::Path2d	mGamma2;
	ci::Path2d	mDelta;
	ci::Path2d	mTheta;
    
	// Colors
	ci::ColorAf	mColorAlpha;
	ci::Colorf	mColorBackground;
	ci::ColorAf mColorBeta;
    ci::ColorAf mColorGamma;
	ci::ColorAf mColorDelta;
	ci::ColorAf mColorTheta;
    
	// Line dimensions
	float		mAmplitude;
	float		mOffset;
	uint32_t	mNumPoints;
	float		mRadius;
	float		mRotation;
	float		mSpeed;
	float		mTrails;
    */
};

#endif // __MINDWAVETEST_H__
