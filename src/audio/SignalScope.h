/*
 *  SignalScope.h
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

class SignalScope : public SubScene
{
public:
    SignalScope(Scene* scene);
    virtual ~SignalScope();

    void setup();
    void setupInterface();
    //void setupDebugInterface();
    void resize();
    void reset();
    void update(double dt);
    void draw();
    
private:
    enum { NUM_POINTS = 512 };
    
    int mNumLines;
    
    float mLineWidth;
    float mFallOffMin;
    float mFallOffMax;
    float mHorizSmoothingMin;
    float mHorizSmoothingMax;
    float mSignalScale;
    float mSignalMaxRatio;
    int mCenterBiasRange;
    bool mRandomizeSignal;
    bool mSmoothLines;
    float mThickness;
    ci::ColorAf mColor;
    
    enum eSignal
    {
        SIGNAL_AMPLITUDE,
        SIGNAL_REAL,
        SIGNAL_IMAGINARY,
        SIGNAL_COUNT
    };
    int mSignalType;
    
    enum eSignalSelection
    {
        SIGNAL_RANDOM,
        SIGNAL_ONE,
    };
    int mSignalSelection;
    
    struct tLine
    {
        tLine() : mValue( NUM_POINTS, 0.0f ) {}
        std::vector<float> mValue;
        ci::PolyLine2f mPolyLine;
    };
    std::vector<tLine> mLines;

};
