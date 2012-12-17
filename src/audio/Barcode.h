/*
 *  Barcode.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#pragma once

#include "SubScene.h"
#include "cinder/Timeline.h"
#include "cinder/PolyLine.h"
#include "cinder/Color.h"

class Scene;

class Barcode : public SubScene
{
public:
    Barcode(Scene* scene);
    virtual ~Barcode();

    void setup();
    void setupInterface();
    void setupDebugInterface();
    void resize();
    void reset();
    void update(double dt);
    void draw();
    
private:
    void drawBar(const int index);
    
    typedef std::function<float (float)> tEaseFn;
    tEaseFn getFalloffFunction();
    
    ci::ColorAf mColor;
    float       mBaseWidth;
    float       mMaxWidth;
    float       mThreshold;
    float       mFalloff;
    
    bool        mWidthByFft;
    bool        mAlphaByFft;
    bool        mPositionByFft;
    bool        mFftFalloff;
    
    int         mNumBars;
    int         mBarGap;
    bool        mVertical;
    
    enum eFftMode
    {
        FFT_ALPHA,
        FFT_WIDTH,
        FFT_POSITION,
        
        FFT_COUNT
    };
    eFftMode mFftMode;
    
    struct tLine
    {
        ci::Anim<float> mWidth;
        ci::Anim<float> mAlpha;
    };
    enum
    {
        MAX_BARS = 64,
        MAX_LINES = 256
    };
    tLine mLines[MAX_BARS][MAX_LINES];
    
    enum eFalloffMode
    {
        FALLOFF_LINEAR,
        FALLOFF_OUTQUAD,
        FALLOFF_OUTEXPO,
        FALLOFF_OUTBACK,
        FALLOFF_OUTBOUNCE,
        FALLOFF_OUTINEXPO,
        FALLOFF_OUTINBACK,
        
        FALLOFF_COUNT
    };
    eFalloffMode mFalloffMode;
};
