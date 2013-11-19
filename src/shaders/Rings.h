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
    
    // callbacks
    enum eColorPresets
    {
        COLOR_NONE,
        COLOR_WHITE,
        COLOR_RED
    };
    bool setRingColor( const int ringIndex, const int colorIndex );
    bool syncParams( const int srcIndex, const int destIndex );
    bool zoomIn( const int ringIndex );
    bool zoomOut( const int ringIndex );
    bool zoomRestore( const int ringIndex );
    
private:

    ci::gl::GlslProg    mShader;
    
    struct tRingSetParams
    {
        ci::Anim<ci::ColorAf>     mColor;
        float           mElapsedTime;
        float           mTimeScale;
        float           mThickness;
        ci::Anim<float>             mFrequency;
        ci::Anim<float>             mScale;
        ci::Anim<float>             mPower;
        ci::Vec2f       mCenter;
        bool            mScaleByAudio;
        bool            mPowerByAudio;
        float           mPrevFrequency;
        float           mPrevPower;
    };
    enum { NUM_RING_SETS = 3 };
    tRingSetParams mRingSetParams[NUM_RING_SETS];
    
    AudioInputHandler mAudioInputHandler;

};