/*
 *  Circles.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */


#include "Circles.h"
#include "Utils.h"
#include "Scene.h"
#include "OculonApp.h"
#include "Interface.h"
#include "Constants.h"
#include "Resources.h"

#include "cinder/gl/gl.h"
#include "cinder/CinderMath.h"
#include "cinder/Rand.h"
#include "cinder/Easing.h"
#include "cinder/ObjLoader.h"

using namespace ci;
using namespace std;


//
// Circles
// 

Circles::Circles(Scene* scene)
: SubScene(scene,"circles")
{
}

Circles::~Circles()
{
}

void Circles::setup()
{
    // params
    mRows = 16;
    mCols = 16;
    mLayers = 1;
    mSpacing = 8.0f;
    mRadius = 32.0f;
    
    mSignalScale = 2.0f;
    
    mColorPrimary = ColorA(1.0f,1.0f,1.0f,1.0f);
    mColorSecondary = ColorA(1.0f,1.0f,1.0f,1.0f);
    
    
    reset();
}

void Circles::setupInterface()
{
    SubScene::setupInterface();
    
    Interface* interface = mParentScene->getInterface();
    
    interface->addParam(CreateIntParam("Rows", &mRows)
                        .minValue(1)
                        .maxValue(16)
                        .oscReceiver(mName,"rows"));
    interface->addParam(CreateIntParam("Cols", &mCols)
                        .minValue(1)
                        .maxValue(16)
                        .oscReceiver(mName,"cols"));
    interface->addParam(CreateFloatParam("Radius", &mRadius)
                        .minValue(1.0f)
                        .maxValue(200.0f)
                        .oscReceiver(mName,"radius"));
    interface->addParam(CreateFloatParam("Spacing", &mSpacing)
                        .minValue(0.0f)
                        .maxValue(100.0f)
                        .oscReceiver(mName,"spacing"));
    interface->addParam(CreateFloatParam("Signal Scale", &mSignalScale)
                        .minValue(1.0f)
                        .maxValue(10.0f)
                        .oscReceiver(mName,"signalscale"));
    
}

void Circles::setupDebugInterface()
{
}

void Circles::reset()
{
}

void Circles::resize()
{
}

void Circles::update(double dt)
{
}

void Circles::draw()
{
    // audio data
    AudioInput& audioInput = mParentScene->getApp()->getAudioInput();
    //float * freqData = audioInput.getFft()->getAmplitude();
    //float * timeData = audioInput.getFft()->getData();
    int32_t dataSize = audioInput.getFft()->getBinSize();
    const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
    // dimensions
    const float windowHeight = mParentScene->getApp()->getViewportHeight();
    const float windowWidth = mParentScene->getApp()->getViewportWidth();
    
    gl::pushMatrices();
    
    gl::color( mColorPrimary );
    
    float totalWidth = mRadius * 2.0f * mCols + mSpacing * (mCols-1);
    float totalHeight = mRadius * 2.0f * mRows + mSpacing * (mRows-1);
    
    float x0 = (windowWidth - totalWidth) / 2.0f + mRadius;
    float y0 = (windowHeight - totalHeight) / 2.0f + mRadius;

    float x = x0;
    float y = y0;
    
    int midRow = mRows / 2;
    int midCol = mCols / 2;
    
    bool evenRows = (mRows % 2) == 0;
    bool evenCols = (mCols % 2) == 0;
    
    float maxRadius = mRadius + mSpacing - 2.0f;
    
    for( int row = 0; row < mRows; ++row )
    {
        for( int col = 0; col < mCols; ++col )
        {
            int fftIndex = 0;
            if( 1 ) // random signal - HACK
            {
                fftIndex = math<int>::max(2,Rand::randInt(dataSize));
            }
            else
            {
                fftIndex = math<int>::abs(midCol-col) + 2;
            }
            float radius = math<float>::max(mRadius/2.0f,fftLogData[fftIndex].y * maxRadius * (1+fftIndex * mSignalScale));
            radius = math<float>::min(radius, maxRadius * 1.2f);
            console() << "fftIndex " << fftIndex << " = " << fftLogData[fftIndex].y << std::endl;
            gl::drawSolidCircle(Vec2f(x,y), radius);
            x += mRadius * 2.0f + mSpacing;
        }
        
        x = x0;
        y += mRadius * 2.0f + mSpacing;
    }
    
    console() << "--------------------------\n";
    
    gl::popMatrices();

}
