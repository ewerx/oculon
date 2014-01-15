//
//  Rings.h
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

#pragma once

#include "AudioInputHandler.h"
#include "Scene.h"
#include "cinder/Cinder.h"
#include "cinder/gl/GlslProg.h"
#include <vector>

//
// Rings
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
    
    bool updateFreq( const int ringIndex );
    bool updateScale( const int ringIndex );
    bool updatePower( const int ringIndex );
    bool updateCenter( const int ringIndex );
    
private:

    ci::gl::GlslProg    mShader;
    
    // format
#define RINGS_FORMAT_TUPLE \
RINGS_FORMAT_ENTRY( "Ring", FORMAT_RING ) \
RINGS_FORMAT_ENTRY( "Vertical", FORMAT_VERTICAL ) \
RINGS_FORMAT_ENTRY( "Horizontal", FORMAT_HORIZONTAL ) \
//end tuple
    
    enum eFormat
    {
#define RINGS_FORMAT_ENTRY( nam, enm ) \
enm,
        RINGS_FORMAT_TUPLE
#undef  RINGS_FORMAT_ENTRY
        
        FORMAT_COUNT
    };
    
    float mAnimTime;
    struct tRingSetParams
    {
        ci::Anim<ci::ColorAf> mColor;
        float           mElapsedTime;
        float           mTimeScale;
        float           mThickness;
        float             mFrequency;
        float             mScale;
        float             mPower;
        ci::Anim<float>             mActualFrequency;
        ci::Anim<float>             mActualScale;
        ci::Anim<float>             mActualPower;
        ci::Vec2f       mCenter;
        ci::Anim<ci::Vec2f> mActualCenter;
        bool            mScaleByAudio;
        bool            mPowerByAudio;
        float           mPrevFrequency;
        float           mPrevPower;
        AudioInputHandler mAudioInputHandler;
        int             mResponseBand;
        
        bool mSeparateByAudio;
        bool mSpin;
        float mSpinRate;
        float mSpinTheta;
        float mSpinRadius;
        eFormat mFormat;
    };
    enum { NUM_RING_SETS = 4 };
    tRingSetParams mRingSetParams[NUM_RING_SETS];
    
    // MUTEK HACKS
    bool mSeparateByAudio;
    bool mSpin;
    float mSpinRate;
    float mSpinTheta;
    float mSpinSeparation;
};