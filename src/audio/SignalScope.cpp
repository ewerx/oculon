/*
 *  SignalScope.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */


#include "SignalScope.h"
#include "Utils.h"
#include "Scene.h"
#include "OculonApp.h"
#include "Interface.h"

#include "cinder/gl/gl.h"
#include "cinder/CinderMath.h"
#include "cinder/Rand.h"
#include "cinder/Easing.h"

using namespace ci;
using namespace std;


//
// SignalScope
// 

SignalScope::SignalScope(Scene* scene)
: SubScene(scene,"signal")
{
}

SignalScope::~SignalScope()
{
}

void SignalScope::setup()
{
    mNumLines = 50;
    mHorizSmoothingMin = 0.4f;
    mHorizSmoothingMax = 0.8f;
    mFallOffMin = 0.75f;
    mFallOffMax = 0.99f;
    mSmoothLines = false;
    mThickness = 2.0f;
    mColor = ColorAf::white();
    
    mCenterBiasRange = 30;
    mSignalScale = 2.0f;
    mSignalMaxRatio = 6.0f;
    
    mRandomizeSignal = true;
    
    mLineWidth = mParentScene->getApp()->getViewportWidth();
    
    reset();
}

void SignalScope::setupInterface()
{
    SubScene::setupInterface();
    
    Interface* interface = mParentScene->getInterface();
    
    interface->addParam(CreateBoolParam("Randomize", &mRandomizeSignal)
                                           .oscReceiver(getName(),"randomize"));
    interface->addParam(CreateFloatParam("Signal Scale", &mSignalScale)
                                           .minValue(0.0f)
                                           .maxValue(10.0f)
                                           .oscReceiver(getName(),"scale")
                                           .sendFeedback());
    interface->addParam(CreateFloatParam("Signal Cap", &mSignalMaxRatio)
                                           .minValue(1.0f)
                                           .maxValue(10.0f)
                                           .oscReceiver(getName(),"cap")
                                           .sendFeedback());
    interface->addParam(CreateIntParam("Center Bias Range", &mCenterBiasRange)
                                           .minValue(1)
                                           .maxValue(NUM_POINTS/4)
                                           .oscReceiver(getName(),"biasrange")
                                           .sendFeedback());
    interface->addParam(CreateFloatParam("Smoothing Min", &mHorizSmoothingMin)
                                           .oscReceiver(getName(),"smoothingmin"));
    interface->addParam(CreateFloatParam("Smoothing Max", &mHorizSmoothingMax)
                                           .oscReceiver(getName(),"smoothingmax"));
    interface->addParam(CreateFloatParam("Falloff Min", &mFallOffMin)
                                           .oscReceiver(getName(),"falloffmin"));
    interface->addParam(CreateFloatParam("Falloff Max", &mFallOffMax)
                                           .oscReceiver(getName(),"falloffmax"));
    interface->addParam(CreateBoolParam("Render Smooth", &mSmoothLines)
                                           .oscReceiver(getName(),"rendersmooth"));
    interface->addParam(CreateFloatParam("Thickness", &mThickness)
                                           .minValue(1.0f)
                                           .maxValue(6.0f)
                                           .oscReceiver(getName(),"thickness"));
    interface->addParam(CreateColorParam("Color", &mColor, kMinColor, kMaxColor)
                                           .oscReceiver(getName(),"color"));
}

//void SignalScope::setupDebugInterface()
//{
//    mParentScene->getDebugInterface()->addSeparator("Signal Scope");
//    mParentScene->getDebugInterface()->setOptions("Falloff Min", "step=0.001");
//    mParentScene->getDebugInterface()->setOptions("Falloff Max", "step=0.001");
//    mParentScene->getDebugInterface()->setOptions("Smoothing Min", "step=0.001");
//    mParentScene->getDebugInterface()->setOptions("Smoothing Max", "step=0.001");
//    
//    mParentScene->getDebugInterface()->setOptions("Center Bias Range", "max=128");
//    mParentScene->getDebugInterface()->setOptions("Signal Cap", "");
//    mParentScene->getDebugInterface()->setOptions("Signal Scale", "step=0.01");
//}

void SignalScope::reset()
{
    mLines = vector<tLine>( mNumLines, tLine() );
}

void SignalScope::resize()
{
}

void SignalScope::update(double dt)
{
    AudioInput& audioInput = mParentScene->getApp()->getAudioInput();
    
    // Get data
    //const float * timeData = audioInput.getFft()->getData();
    vector<float> amplitude = audioInput.getMagSpectrum();
    const int32_t	binCount = audioInput.getFftBandCount();
//    const float *	imaginary = audioInput.getFft()->getImaginary();
//    //const float *	phase = audioInput.getFft()->getPhase();
//    const float *	real = audioInput.getFft()->getReal();
//    const int32_t dataSize = audioInput.getFft()->getBinSize();
    //const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
//    assert( binSize < NUM_POINTS );
    
    // Get dimensions
    const float windowHeight = mParentScene->getApp()->getViewportHeight();
    const float windowWidth = mParentScene->getApp()->getViewportWidth();
    
    const float scaleX = mLineWidth / (float)binCount;
    const float xOffset = (windowWidth - mLineWidth) / 2.0f; 
    
    const float gap = windowHeight / (mNumLines+1);
    float scaleY = gap * 10.f;
    float cap = gap * mSignalMaxRatio;
    float yOffset = windowHeight - gap;
    
    
    
    Rand indexRand;
    float adjacentY[mNumLines][binCount];
    //const float threshold = 0.0000015f;
    
    for( int lineIndex=0; lineIndex < mNumLines; ++lineIndex )
    {
        mLines[lineIndex].mPolyLine.getPoints().clear();
        
        int signalType = Rand::randInt(SIGNAL_COUNT);
        
        int center = (binCount/2) + mCenterBiasRange * Rand::randFloat(-1.0f,1.0f);
        
        float falloff = Rand::randFloat(mFallOffMin,mFallOffMax);
        float horizSmoothing = Rand::randFloat(mHorizSmoothingMin,mHorizSmoothingMax);
        
        indexRand.seed(lineIndex);
        for( int32_t i=0; i < binCount; ++i )
        {
            // center bias
            float centerRatio = (center - math<float>::abs( center - i )) / center;
            float centerBias = easeInExpo(centerRatio);
            float scaleFactor = centerBias * mSignalScale;
            
            int bandIndex = mRandomizeSignal ? Rand::randInt(binCount) : indexRand.nextInt(binCount);
            
            float signal = 0.0f;
            switch(signalType)
            {
                case SIGNAL_AMPLITUDE:
                    signal = amplitude[bandIndex];
                    break;
//                case SIGNAL_IMAGINARY:
//                    signal = imaginary[bandIndex];
//                    break;
//                case SIGNAL_REAL:
//                    signal = real[bandIndex];
//                    break;
            }
            
            float y = signal * scaleFactor * scaleY;
            
            float min = 0.0f;
            if( i > 0 && i < binCount-1)
            {
                min = mLines[lineIndex].mValue[i] * falloff;
                
                // smooth height diff of point to the left
                float damp = mLines[lineIndex].mValue[i-1] * horizSmoothing;
                
                min = math<float>::max( min, damp );
            }
            y = math<float>::clamp( y, min, cap );
            
            mLines[lineIndex].mValue[i] = y;
        }
        
        for( int32_t i=0; i < binCount; ++i ) 
        {
            // smoothen height diff of point to the right
            float damp = mLines[lineIndex].mValue[i+1] * horizSmoothing;
            
            mLines[lineIndex].mValue[i] = math<float>::clamp( mLines[lineIndex].mValue[i], damp, cap );
            
            float absY = yOffset - mLines[lineIndex].mValue[i];
            
            // compare to line below, keep from crossing over
            if( lineIndex > 0 && adjacentY[lineIndex-1][i] < absY )
            {
                absY = adjacentY[lineIndex-1][i];
                //y = mLines[lineIndex-1].mValue[i];
            }
            adjacentY[lineIndex][i] = absY;
            
            mLines[lineIndex].mPolyLine.push_back(Vec2f(i * scaleX + xOffset, absY));
        }

        yOffset -= gap;
    } 
}

void SignalScope::draw()
{
    if( mSmoothLines )
    {
        gl::enable( GL_LINE_SMOOTH );
        glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    }
    else
    {
        gl::disable( GL_LINE_SMOOTH );
    }
    glLineWidth(mThickness);
    
    gl::color( mColor );
    
    for( int lineIndex=0; lineIndex < mNumLines; ++lineIndex )
    {
        gl::draw(mLines[lineIndex].mPolyLine);
    }

}
