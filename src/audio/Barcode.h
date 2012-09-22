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
    
    
    ci::ColorAf mColor;
    float       mBaseWidth;
    float       mMaxWidth;
    float       mThreshold;
    
    bool        mWidthByFft;
    bool        mAlphaByFft;
    bool        mPositionByFft;
    
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
};
