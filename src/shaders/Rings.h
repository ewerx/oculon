//
//  Rings.h
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

#pragma once

#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "Scene.h"
#include "MotionBlurRenderer.h"
#include "GridRenderer.h"
#include "AudioInputHandler.h"

#include "OscMessage.h"

//
// Audio input tests
//
class Rings : public Scene
{
public:
    Rings();
    virtual ~Rings();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    
protected:// from Scene
    void setupInterface();
    //void setupDebugInterface();
    
private:
    void drawScene();
    
    void shaderPreDraw();
    void drawShaderOutput();
    void shaderPostDraw();
    
private:

    ci::gl::GlslProg    mShader;
    
    std::vector<ci::gl::VboMesh> mRings;
    
    // audio
    //AudioInputHandler   mAudioInputHandler;
    
    // params
    ci::ColorAf         mColor1;
    ci::ColorAf         mColor2;
    ci::ColorAf         mColor3;
    float               mTimeScale;
    double              mElapsedTime;
    
    ci::Vec3f           mThickness;
    ci::Vec3f           mFrequency;

//    enum { NUM_RING_SETS = 3 };
//    tRingSetParams mRingSetParams[NUM_RING_SETS];

    int                 mNumRings;
    float               mScale;
    bool                mAudioScale;
    int                 mIntervals;
    int                 mColorMode;
    ci::Vec3f           mCoefficients;
};

