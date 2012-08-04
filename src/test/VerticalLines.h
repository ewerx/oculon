/*
 *  VerticalLines.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#pragma once

#include "cinder/Timeline.h"
#include "cinder/PolyLine.h"
#include "cinder/Color.h"

class Scene;

//TODO: refactor as SubScene
class VerticalLines
{
public:
    VerticalLines(Scene* scene);
    virtual ~VerticalLines();

    void setup();
    void setupInterface();
    void setupDebugInterface();
    void resize();
    void reset();
    void update(double dt);
    void draw();
    
private:
    ci::ColorAf mColor;
    float       mBaseWidth;
    float       mMaxWidth;
    float       mIntensity;
    float       mThreshold;
    
    bool        mWidthByFft;
    bool        mAlphaByFft;
    bool        mPositionByFft;
    
    enum eFftMode
    {
        FFT_ALPHA,
        FFT_WIDTH,
        FFT_POSITION,
        
        FFT_COUNT
    };
    eFftMode mFftMode;
    
    Scene*  mParentScene;

};
