/*
 *  Circles.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#pragma once

#include "SubScene.h"
#include "cinder/Timeline.h"
#include "cinder/Color.h"
#include <boost/multi_array.hpp>

class Scene;

class Circles : public SubScene
{
public:
    Circles(Scene* scene);
    virtual ~Circles();

    void setup();
    void setupInterface();
    void setupDebugInterface();
    void resize();
    void reset();
    void update(double dt);
    void draw();
    
    //callbacks
    bool baseRadiusChanged();
    
private:
    
    typedef std::function<float (float)> tEaseFn;
    tEaseFn getFalloffFunction();
    
    struct tCircle
    {
        ci::Anim<float> mRadius;
        ci::Anim<float> mAlpha;
    };
    enum {
        MAX_X = 256,
        MAX_Y = 256
    };
    tCircle mCircles[MAX_X][MAX_Y];
    
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
    
    //TODO: boost multi_array?
//    typedef boost::multi_array<struct tCircle,2> tCircleArray;
//    tCircleArray mCircles(boost::extents[MAX_X][MAX_Y]);
    
    int mRows;
    int mCols;
    int mLayers;
    
    float mSpacing;
    float mBaseRadius;
    float mFalloff;
    
    bool mAlphaByAudio;
    
    float mSignalScale;
    
    ci::ColorA mColorPrimary;
    ci::ColorA mColorSecondary;

};
