/*
 *  Eclipse.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */


#include "Eclipse.h"
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
// Eclipse
// 

Eclipse::Eclipse(Scene* scene)
: mParentScene(scene)
{
}

Eclipse::~Eclipse()
{
}

void Eclipse::setup()
{
    mShowTestBackground = false;
    mTestBackground = ci::loadImage( ci::loadFile( "/Users/ehsan/Downloads/eclipse_mapping_pattern.jpg" ) );
    
    mDim = 168; // cube dimension
    mXOffset = 8;
    mYOffset = 130;
    
    mMode = MODE_LINES;
    
    reset();
}

void Eclipse::setupInterface()
{
    Interface* interface = mParentScene->getInterface();
    const string name("eclipse");
    interface->gui()->addColumn();
    interface->gui()->addLabel("Eclipse Cubes");
    interface->addParam(CreateBoolParam("Test Pattern", &mShowTestBackground)
                        .oscReceiver(name,"testpattern"));
    interface->addEnum(CreateEnumParam("Draw Mode", (int*)&mMode)
                       .maxValue(MODE_COUNT)
                       .oscReceiver(name,"drawmode"));
}

void Eclipse::setupDebugInterface()
{
}

void Eclipse::reset()
{
}

void Eclipse::resize()
{
}

void Eclipse::update(double dt)
{
    gl::enable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    gl::enable( GL_POLYGON_SMOOTH );
    glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    gl::color( ColorAf::white() );
    glLineWidth(1.0f);
    
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
    
        
}

void Eclipse::draw()
{
    AudioInput& audioInput = mParentScene->getApp()->getAudioInput();
    const float * amplitude = audioInput.getFft()->getAmplitude();
    const int32_t	binSize = audioInput.getFft()->getBinSize();

    
    gl::enable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    gl::enable( GL_POLYGON_SMOOTH );
    glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    gl::color( ColorAf::white() );
    glLineWidth(4.0f);
    
    const float windowHeight = mParentScene->getApp()->getViewportHeight();
    const float windowWidth = mParentScene->getApp()->getViewportWidth();
    
    const float centerX = windowWidth/2.0f;
    const float centerY = windowHeight/2.0f;
    
    //gl::enableAdditiveBlending();
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    if( mShowTestBackground )
        gl::draw( mTestBackground );
    
    switch( mMode )
    {
        case MODE_LINES:
            drawLines();
            break;
        
        case MODE_CUBES:
            drawCubes();
            break;
            
        default:
            break;
    }
}

void Eclipse::drawLines()
{
    int x = mXOffset;
    int y = mYOffset;
    const float halfDim = mDim/2.0f;
    
    // row 1
    for( int i=0; i < ROW1_RECTS; ++i )
    {
        gl::drawStrokedRect(Rectf(x+mDim*i,y,x+mDim*(i+1),y+mDim));
    }
    // row 2
    x += mDim;
    y += mDim;
    for( int i=0; i < ROW2_RECTS; ++i )
    {
        gl::drawStrokedRect(Rectf(x+mDim*i,y,x+mDim*(i+1),y+mDim));
    }
    // row 3
    x += mDim;
    y += mDim;
    for( int i=0; i < ROW3_RECTS; ++i )
    {
        gl::drawStrokedRect(Rectf(x+mDim*i,y,x+mDim*(i+1),y+mDim));
    }
}

void Eclipse::drawCubes()
{
    gl::pushMatrices();
    
    int x = mXOffset;
    int y = mYOffset;
    const float halfDim = mDim/2.0f;
    const Vec3f dimVec = Vec3f(mDim,mDim,mDim);
    
    gl::translate(x+halfDim,y+halfDim,-halfDim);
    // row 1
    gl::pushMatrices();
    for( int i=0; i < ROW1_RECTS; ++i )
    {
        gl::pushMatrices();
        gl::rotate(Vec3f(0.0f,getElapsedSeconds()*10.0f,0.0f));
        gl::drawStrokedCube(Vec3f(0.0f,0.0f,0.0f), dimVec);
        gl::popMatrices();
        gl::translate(mDim, 0.0f, 0.0f);
    }
    gl::popMatrices();
    
    // row 2
    gl::translate(mDim,mDim,0.0f);
    gl::pushMatrices();
    for( int i=0; i < ROW2_RECTS; ++i )
    {
        gl::pushMatrices();
        gl::rotate(Vec3f(0.0f,getElapsedSeconds()*-1.0f,0.0f));
        gl::drawStrokedCube(Vec3f(0.0f,0.0f,0.0f), dimVec);
        gl::popMatrices();
        gl::translate(mDim, 0.0f, 0.0f);
    }
    gl::popMatrices();
    
    // row 3
    gl::translate(mDim,mDim,0.0f);
    gl::pushMatrices();
    for( int i=0; i < ROW3_RECTS; ++i )
    {
        gl::pushMatrices();
        gl::rotate(Vec3f(0.0f,getElapsedSeconds()*50.0f,0.0f));
        gl::drawStrokedCube(Vec3f(0.0f,0.0f,0.0f), dimVec);
        gl::popMatrices();
        gl::translate(mDim, 0.0f, 0.0f);
    }
    gl::popMatrices();
    
    gl::popMatrices();
}