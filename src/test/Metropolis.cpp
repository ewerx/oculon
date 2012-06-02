/*
 *  Metropolis.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */


#include "Metropolis.h"
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
// Metropolis
// 

Metropolis::Metropolis(Scene* scene)
: mParentScene(scene)
{
}

Metropolis::~Metropolis()
{
}

void Metropolis::setup()
{
    mNumLines = 50;
    mHorizSmoothingMin = 0.4f;
    mHorizSmoothingMax = 0.8f;
    mFallOffMin = 0.75f;
    mFallOffMax = 0.99f;
    
    mCenterBiasRange = 30;
    mSignalScale = 2.0f;
    mSignalMaxRatio = 6.0f;
    
    mRandomizeSignal = true;
    
    mLineWidth = mParentScene->getApp()->getViewportWidth();
    
    mTestBackground = ci::loadImage( ci::loadFile( "/Users/ehsan/Desktop/mutek_metropolis_canvas.png" ) ); 
    
    reset();
}

void Metropolis::setupInterface()
{
    const string name("metropolis");
    mParentScene->getInterface()->gui()->addColumn();
    mParentScene->getInterface()->addParam(CreateBoolParam("Randomize", &mRandomizeSignal)
                                           .oscReceiver(name,"randomize"));
    mParentScene->getInterface()->addParam(CreateFloatParam("Signal Scale", &mSignalScale)
                                           .minValue(0.0f)
                                           .maxValue(10.0f)
                                           .oscReceiver(name,"scale")
                                           .sendFeedback());
    mParentScene->getInterface()->addParam(CreateFloatParam("Signal Cap", &mSignalMaxRatio)
                                           .minValue(1.0f)
                                           .maxValue(10.0f)
                                           .oscReceiver(name,"cap")
                                           .sendFeedback());
    mParentScene->getInterface()->addParam(CreateIntParam("Center Bias Range", &mCenterBiasRange)
                                           .minValue(1)
                                           .maxValue(NUM_POINTS/2)
                                           .oscReceiver(name,"biasrange")
                                           .sendFeedback());
    mParentScene->getInterface()->addParam(CreateFloatParam("Smoothing Min", &mHorizSmoothingMin)
                                           .oscReceiver(name,"smoothingmin"));
    mParentScene->getInterface()->addParam(CreateFloatParam("Smoothing Max", &mHorizSmoothingMax)
                                           .oscReceiver(name,"smoothingmax"));
    mParentScene->getInterface()->addParam(CreateFloatParam("Falloff Min", &mFallOffMin)
                                           .oscReceiver(name,"falloffmin"));
    mParentScene->getInterface()->addParam(CreateFloatParam("Falloff Max", &mFallOffMax)
                                           .oscReceiver(name,"falloffmax"));
}

void Metropolis::setupDebugInterface()
{
    mParentScene->getDebugInterface()->addSeparator("Signal Scope");
    mParentScene->getDebugInterface()->setOptions("Falloff Min", "step=0.001");
    mParentScene->getDebugInterface()->setOptions("Falloff Max", "step=0.001");
    mParentScene->getDebugInterface()->setOptions("Smoothing Min", "step=0.001");
    mParentScene->getDebugInterface()->setOptions("Smoothing Max", "step=0.001");
    
    mParentScene->getDebugInterface()->setOptions("Center Bias Range", "max=128");
    mParentScene->getDebugInterface()->setOptions("Signal Cap", "");
    mParentScene->getDebugInterface()->setOptions("Signal Scale", "step=0.01");
}

void Metropolis::reset()
{
    mLines = vector<tLine>( mNumLines, tLine() );
}

void Metropolis::resize()
{
    
}

void Metropolis::update(double dt)
{
    AudioInput& audioInput = mParentScene->getApp()->getAudioInput();
    
    // Get data
    const float * timeData = audioInput.getFft()->getData();
    const float * amplitude = audioInput.getFft()->getAmplitude();
    const int32_t	binSize = audioInput.getFft()->getBinSize();
    const float *	imaginary = audioInput.getFft()->getImaginary();
    const float *	phase = audioInput.getFft()->getPhase();
    const float *	real = audioInput.getFft()->getReal();
    const int32_t dataSize = audioInput.getFft()->getBinSize();
    const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
    assert( binSize < NUM_POINTS );
    
    // Get dimensions
    const float windowHeight = mParentScene->getApp()->getViewportHeight();
    const float windowWidth = mParentScene->getApp()->getViewportWidth();
    
    const float scaleX = mLineWidth / (float)dataSize;
    const float xOffset = (windowWidth - mLineWidth) / 2.0f; 
    
    const float gap = windowHeight / (mNumLines+1);
    float scaleY = gap * 10.f;
    float cap = gap * mSignalMaxRatio;
    float yOffset = windowHeight - gap;
    
    
    
    /*
    Rand indexRand;
    float adjacentY[mNumLines][binSize];
    //const float threshold = 0.0000015f;
    
    for( int lineIndex=0; lineIndex < mNumLines; ++lineIndex )
    {
        mLines[lineIndex].mPolyLine.getPoints().clear();
        
        int signalType = Rand::randInt(SIGNAL_COUNT);
        
        int center = (binSize/2) + mCenterBiasRange * Rand::randFloat(-1.0f,1.0f);
        
        float falloff = Rand::randFloat(mFallOffMin,mFallOffMax);
        float horizSmoothing = Rand::randFloat(mHorizSmoothingMin,mHorizSmoothingMax);
        
        indexRand.seed(lineIndex);
        for( int32_t i=0; i < binSize; ++i ) 
        {
            // center bias
            float centerRatio = (center - math<float>::abs( center - i )) / center;
            float centerBias = easeInExpo(centerRatio);
            float scaleFactor = centerBias * mSignalScale;
            
            int bandIndex = mRandomizeSignal ? Rand::randInt(binSize) : indexRand.nextInt(binSize);
            
            float signal = 0.0f;
            switch(signalType)
            {
                case SIGNAL_AMPLITUDE:
                    signal = amplitude[bandIndex];
                    break;
                case SIGNAL_IMAGINARY:
                    signal = imaginary[bandIndex];
                    break;
                case SIGNAL_REAL:
                    signal = real[bandIndex];
                    break;
            }
            
            float y = signal * scaleFactor * scaleY;
            
            float min = 0.0f;
            if( i > 0 && i < binSize-1)
            {
                min = mLines[lineIndex].mValue[i] * falloff;
                
                // smooth height diff of point to the left
                float damp = mLines[lineIndex].mValue[i-1] * horizSmoothing;
                
                min = math<float>::max( min, damp );
            }
            y = math<float>::clamp( y, min, cap );
            
            mLines[lineIndex].mValue[i] = y;
        }
        
        for( int32_t i=0; i < binSize; ++i ) 
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
        
        //gl::color( Color(1.0f,1.0f,1.0f) );
        
        //gl::color( Color(1.0f,0.0f,0.0f) );
        //gl::draw(testLine);
        yOffset -= gap;
    } 
     */
}

void Metropolis::draw()
{
    gl::enable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    gl::enable( GL_POLYGON_SMOOTH );
    glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    gl::color( ColorAf::white() );
    
    
    const float windowHeight = mParentScene->getApp()->getViewportHeight();
    const float windowWidth = mParentScene->getApp()->getViewportWidth();
    
    const float centerX = windowWidth/2.0f;
    const float centerY = windowHeight/2.0f;
    
    // grid
    const int numSegs = 14;
    const float segWidth = windowWidth / numSegs;
    
    /*
    glColor4f(1.0f, 1.0f, 1.0f, 0.75f);
    gl::drawLine( Vec2f(0.0f, windowHeight/3.0f), Vec2f(windowWidth, windowHeight/3.0f) );
    gl::drawLine( Vec2f(0.0f, windowHeight*2.0f/3.0f), Vec2f(windowWidth, windowHeight*2.0f/3.0f) );
    
    float x = segWidth / 2.0f;
    
    for( int i=0; i < numSegs; ++i )
    {
        gl::drawLine( Vec2f(x, 0.0f), Vec2f(x, windowHeight) );
        x += segWidth;
    }
    */
    gl::enableAdditiveBlending();
    gl::disableDepthWrite();
    gl::disableDepthRead();
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    gl::draw( mTestBackground );
    
    glColor4f(1.0f, 0.0f, 0.0f, 0.75f);
    
    gl::drawLine( Vec2f(centerX, 0.0f), Vec2f(centerX, windowHeight) );
    gl::drawLine( Vec2f(0.0f, centerY), Vec2f(windowWidth, centerY) );
    
    const float c1 = 5.2f*segWidth;
    gl::drawLine( Vec2f(centerX, centerY), Vec2f(centerX + c1, 0.0f) );
    gl::drawLine( Vec2f(centerX, centerY), Vec2f(centerX - c1, 0.0f) );
    gl::drawLine( Vec2f(centerX, centerY), Vec2f(centerX + c1, windowHeight) );
    gl::drawLine( Vec2f(centerX, centerY), Vec2f(centerX - c1, windowHeight) );
    
    const float c2 = 1.7f*segWidth;
    gl::drawLine( Vec2f(centerX, centerY), Vec2f(centerX + c2, 0.0f) );
    gl::drawLine( Vec2f(centerX, centerY), Vec2f(centerX - c2, 0.0f) );
    gl::drawLine( Vec2f(centerX, centerY), Vec2f(centerX + c2, windowHeight) );
    gl::drawLine( Vec2f(centerX, centerY), Vec2f(centerX - c2, windowHeight) );
    
    glColor4f(1.0f, 0.0f, 0.0f, 0.75f);
    
    const float s1 = 0.0f - segWidth*0.8f;
    const float s2 = windowWidth + segWidth*0.8f;
    
    const float ps1 = 6.15*segWidth;
    const float ps2 = 4.9f*segWidth;
    const float ps3 = 2.7f*segWidth;
    const float ps4 = -3.3f*segWidth;
    
    gl::drawLine( Vec2f(s2, centerY), Vec2f(centerX + ps1, 0.0f) );
    gl::drawLine( Vec2f(s1, centerY), Vec2f(centerX - ps1, 0.0f) );
    gl::drawLine( Vec2f(s2, centerY), Vec2f(centerX + ps2, 0.0f) );
    gl::drawLine( Vec2f(s1, centerY), Vec2f(centerX - ps2, 0.0f) );
    gl::drawLine( Vec2f(s2, centerY), Vec2f(centerX + ps3, 0.0f) );
    gl::drawLine( Vec2f(s1, centerY), Vec2f(centerX - ps3, 0.0f) );
    gl::drawLine( Vec2f(s2, centerY), Vec2f(centerX + ps4, 0.0f) );
    gl::drawLine( Vec2f(s1, centerY), Vec2f(centerX - ps4, 0.0f) );
    gl::drawLine( Vec2f(s2, centerY), Vec2f(centerX + ps1, windowHeight) );
    gl::drawLine( Vec2f(s1, centerY), Vec2f(centerX - ps1, windowHeight) );
    gl::drawLine( Vec2f(s2, centerY), Vec2f(centerX + ps2, windowHeight) );
    gl::drawLine( Vec2f(s1, centerY), Vec2f(centerX - ps2, windowHeight) );
    gl::drawLine( Vec2f(s2, centerY), Vec2f(centerX + ps3, windowHeight) );
    gl::drawLine( Vec2f(s1, centerY), Vec2f(centerX - ps3, windowHeight) );
    gl::drawLine( Vec2f(s2, centerY), Vec2f(centerX + ps4, windowHeight) );
    gl::drawLine( Vec2f(s1, centerY), Vec2f(centerX - ps4, windowHeight) );
    
    glColor4f(1.0f, 1.0f, 0.0f, 0.75f);
    
    const float v1 = 3.925f*segWidth;
    gl::drawLine( Vec2f(centerX+v1, 0.0f), Vec2f(centerX+v1, windowHeight) );
    gl::drawLine( Vec2f(centerX-v1, 0.0f), Vec2f(centerX-v1, windowHeight) );
    const float v2 = 0.2f*segWidth;
    gl::drawLine( Vec2f(v2, 0.0f), Vec2f(v2, windowHeight) );
    gl::drawLine( Vec2f(windowWidth-v2, 0.0f), Vec2f(windowWidth-v2, windowHeight) );
    
    const float m1 = windowHeight / 6.2f;
    const float pm1 = 4.95f*segWidth;
    const float pm2 = 2.8f*segWidth;
    const float pm3 = -0.15f*segWidth;
    gl::drawLine( Vec2f(v2, -m1), Vec2f(centerX - pm1, windowHeight) );
    gl::drawLine( Vec2f(v2, -m1), Vec2f(centerX - pm2, windowHeight) );
    gl::drawLine( Vec2f(v2, -m1), Vec2f(centerX - pm3, windowHeight) );
    
    gl::drawLine( Vec2f(v2, windowHeight+m1), Vec2f(centerX - pm1, 0.0f) );
    gl::drawLine( Vec2f(v2, windowHeight+m1), Vec2f(centerX - pm2, 0.0f) );
    gl::drawLine( Vec2f(v2, windowHeight+m1), Vec2f(centerX - pm3, 0.0f) );
    
    gl::drawLine( Vec2f(windowWidth-v2, -m1), Vec2f(centerX + pm1, windowHeight) );
    gl::drawLine( Vec2f(windowWidth-v2, -m1), Vec2f(centerX + pm2, windowHeight) );
    gl::drawLine( Vec2f(windowWidth-v2, -m1), Vec2f(centerX + pm3, windowHeight) );
    
    gl::drawLine( Vec2f(windowWidth-v2, windowHeight+m1), Vec2f(centerX + pm1, 0.0f) );
    gl::drawLine( Vec2f(windowWidth-v2, windowHeight+m1), Vec2f(centerX + pm2, 0.0f) );
    gl::drawLine( Vec2f(windowWidth-v2, windowHeight+m1), Vec2f(centerX + pm3, 0.0f) );
    
    const float pm4 = 1.3f*segWidth;
    const float pm5 = 2.875f*segWidth;
    gl::drawLine( Vec2f(centerX-pm5, 0.0f), Vec2f(centerX - pm4, windowHeight) );
    gl::drawLine( Vec2f(centerX-pm4, 0.0f), Vec2f(centerX - pm5, windowHeight) );
    gl::drawLine( Vec2f(centerX+pm5, 0.0f), Vec2f(centerX + pm4, windowHeight) );
    gl::drawLine( Vec2f(centerX+pm4, 0.0f), Vec2f(centerX + pm5, windowHeight) );
    /*
    for( int lineIndex=0; lineIndex < mNumLines; ++lineIndex )
    {
        gl::draw(mLines[lineIndex].mPolyLine);
    }
     */
}
