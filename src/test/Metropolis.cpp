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
    
    mMetropolis.init( mParentScene->getApp()->getViewportWidth(), mParentScene->getApp()->getViewportHeight() );
    
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
    gl::enable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    gl::enable( GL_POLYGON_SMOOTH );
    glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    gl::color( ColorAf::white() );
    glLineWidth(2.0f);
    
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
    
}

//void Metropolis::drawTriangle( const Vec2f& origin, const Vec2f& farPoint

void Metropolis::draw()
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
    //glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    //gl::draw( mTestBackground );
    
    glColor4f(1.0f, 0.0f, 0.0f, 0.75f);
    
    gl::drawLine( Vec2f(centerX, 0.0f), Vec2f(centerX, windowHeight) );
    gl::drawLine( Vec2f(0.0f, centerY), Vec2f(windowWidth, centerY) );
    
    // fixed points
    const float centerSidePointX = mMetropolis.centerSidePointX;
    const float topBotSidePointX = mMetropolis.centerSidePointX;
    const float topBotSidePointY = mMetropolis.topBotSidePointY;
    const Vec2f p0(mMetropolis.p0); // center
    const Vec2f p1(mMetropolis.p1); // left side
    const Vec2f p2(windowWidth - centerSidePointX, centerY); // right side
    const Vec2f p3(topBotSidePointX, topBotSidePointY); // top left
    const Vec2f p4(topBotSidePointX, windowHeight - topBotSidePointY); // bottom left
    const Vec2f p5(windowWidth - topBotSidePointX, topBotSidePointY); // top right
    const Vec2f p6(windowWidth - topBotSidePointX, windowHeight - topBotSidePointY); // bottom right
    
    
    // p0
    const float c0 = centerX;
    const float c1 = 5.2f*segWidth;
    const float c2 = 1.7f*segWidth;
    const float c3 = FLT_MAX;
    gl::drawLine( p0, Vec2f(centerX + c1, 0.0f) );
    gl::drawLine( p0, Vec2f(centerX - c1, 0.0f) );
    gl::drawLine( p0, Vec2f(centerX + c1, windowHeight) );
    gl::drawLine( p0, Vec2f(centerX - c1, windowHeight) );
    
    gl::drawLine( p0, Vec2f(centerX + c2, 0.0f) );
    gl::drawLine( p0, Vec2f(centerX - c2, 0.0f) );
    gl::drawLine( p0, Vec2f(centerX + c2, windowHeight) );
    gl::drawLine( p0, Vec2f(centerX - c2, windowHeight) );
    
    float points[3];
    points[0] = Rand::randFloat(0.0f,c2);
    points[1] = Rand::randFloat(c2,c1);
    points[2] = Rand::randFloat(c1,c3);
    //points[3] = Rand::randFloat(-FLT_MAX,ps4);
    
    
    for( int i=0; i < 3; ++i )
    {
        /*
        glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 0.0f, 0.0f, 0.75f);
        glVertex2f( p0.x, p0.y );
        glVertex2f( p0.x, 0.0f );
        glColor4f(0.0f,0.0f,0.0f,0.0f);
        glVertex2f( centerX - points[i], 0.0f );
        glEnd();
        
        glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 0.0f, 0.0f, 0.75f);
        glVertex2f( p0.x, p0.y );
        glVertex2f( p0.x, 0.0f );
        glColor4f(0.0f,0.0f,0.0f,0.0f);
        glVertex2f( centerX + points[i], 0.0f );
        glEnd();
        
        glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 0.0f, 0.0f, 0.75f);
        glVertex2f( p0.x, p0.y );
        glVertex2f( p0.x, windowHeight );
        glColor4f(0.0f,0.0f,0.0f,0.0f);
        glVertex2f( centerX - points[i], windowHeight );
        //glEnd();
        
        glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 0.0f, 0.0f, 0.75f);
        glVertex2f( p0.x, p0.y );
        glVertex2f( p0.x, windowHeight );
        glColor4f(0.0f,0.0f,0.0f,0.0f);
        glVertex2f( centerX + points[i], windowHeight );
        glEnd();
        */
        gl::drawLine( p0, Vec2f(centerX - points[i], 0.0f) );
        gl::drawLine( p0, Vec2f(centerX + points[i], 0.0f) );
        gl::drawLine( p0, Vec2f(centerX - points[i], windowHeight) );
        gl::drawLine( p0, Vec2f(centerX + points[i], windowHeight) );
    }
    
    // p1 + p2
    glColor4f(1.0f, 0.0f, 0.0f, 0.75f);
    
    
    const float ps1 = 6.15*segWidth;
    const float ps2 = 4.9f*segWidth;
    const float ps3 = 2.7f*segWidth;
    const float ps4 = -3.3f*segWidth;
    
    
    //float points[3];
    points[0] = Rand::randFloat(ps1,ps2);
    points[1] = Rand::randFloat(ps2,ps3);
    points[2] = Rand::randFloat(ps4,ps3);
    points[3] = Rand::randFloat(-FLT_MAX,ps4);
    
    for( int i=0; i < 3; ++i )
    {
        gl::drawLine( p1, Vec2f(centerX - points[i], 0.0f) );
        gl::drawLine( p2, Vec2f(centerX + points[i], 0.0f) );
        gl::drawLine( p1, Vec2f(centerX - points[i], windowHeight) );
        gl::drawLine( p2, Vec2f(centerX + points[i], windowHeight) );
    }
    
    gl::drawLine( p2, Vec2f(centerX + ps1, 0.0f) );
    gl::drawLine( p1, Vec2f(centerX - ps1, 0.0f) );
    gl::drawLine( p2, Vec2f(centerX + ps2, 0.0f) );
    gl::drawLine( p1, Vec2f(centerX - ps2, 0.0f) );
    gl::drawLine( p2, Vec2f(centerX + ps3, 0.0f) );
    gl::drawLine( p1, Vec2f(centerX - ps3, 0.0f) );
    gl::drawLine( p2, Vec2f(centerX + ps4, 0.0f) );
    gl::drawLine( p1, Vec2f(centerX - ps4, 0.0f) );
    gl::drawLine( p2, Vec2f(centerX + ps1, windowHeight) );
    gl::drawLine( p1, Vec2f(centerX - ps1, windowHeight) );
    gl::drawLine( p2, Vec2f(centerX + ps2, windowHeight) );
    gl::drawLine( p1, Vec2f(centerX - ps2, windowHeight) );
    gl::drawLine( p2, Vec2f(centerX + ps3, windowHeight) );
    gl::drawLine( p1, Vec2f(centerX - ps3, windowHeight) );
    gl::drawLine( p2, Vec2f(centerX + ps4, windowHeight) );
    gl::drawLine( p1, Vec2f(centerX - ps4, windowHeight) );
    
    /*
    gl::drawLine( p2, Vec2f(centerX + ps1, 0.0f) );
    gl::drawLine( p1, Vec2f(centerX - ps1, 0.0f) );
    gl::drawLine( p2, Vec2f(centerX + ps2, 0.0f) );
    gl::drawLine( p1, Vec2f(centerX - ps2, 0.0f) );
    gl::drawLine( p2, Vec2f(centerX + ps3, 0.0f) );
    gl::drawLine( p1, Vec2f(centerX - ps3, 0.0f) );
    gl::drawLine( p2, Vec2f(centerX + ps4, 0.0f) );
    gl::drawLine( p1, Vec2f(centerX - ps4, 0.0f) );
    gl::drawLine( p2, Vec2f(centerX + ps1, windowHeight) );
    gl::drawLine( p1, Vec2f(centerX - ps1, windowHeight) );
    gl::drawLine( p2, Vec2f(centerX + ps2, windowHeight) );
    gl::drawLine( p1, Vec2f(centerX - ps2, windowHeight) );
    gl::drawLine( p2, Vec2f(centerX + ps3, windowHeight) );
    gl::drawLine( p1, Vec2f(centerX - ps3, windowHeight) );
    gl::drawLine( p2, Vec2f(centerX + ps4, windowHeight) );
    gl::drawLine( p1, Vec2f(centerX - ps4, windowHeight) );
     */
    
    glColor4f(1.0f, 1.0f, 1.0f, 0.75f);
    // p3-p6
    const float pm1 = 4.95f*segWidth;
    const float pm2 = 2.8f*segWidth;
    const float pm3 = -0.15f*segWidth;
    gl::drawLine( p3, Vec2f(centerX - pm1, windowHeight) );
    gl::drawLine( p3, Vec2f(centerX - pm2, windowHeight) );
    gl::drawLine( p3, Vec2f(centerX - pm3, windowHeight) );
    gl::drawLine( p3, p4 );
    
    gl::drawLine( p4, Vec2f(centerX - pm1, 0.0f) );
    gl::drawLine( p4, Vec2f(centerX - pm2, 0.0f) );
    gl::drawLine( p4, Vec2f(centerX - pm3, 0.0f) );
    gl::drawLine( p4, p3 );
    
    gl::drawLine( p5, Vec2f(centerX + pm1, windowHeight) );
    gl::drawLine( p5, Vec2f(centerX + pm2, windowHeight) );
    gl::drawLine( p5, Vec2f(centerX + pm3, windowHeight) );
    gl::drawLine( p5, p6 );
    
    gl::drawLine( p6, Vec2f(centerX + pm1, 0.0f) );
    gl::drawLine( p6, Vec2f(centerX + pm2, 0.0f) );
    gl::drawLine( p6, Vec2f(centerX + pm3, 0.0f) );
    gl::drawLine( p6, p5 );
    
    // p7-p10
    const float v1 = 3.925f*segWidth;
    gl::drawLine( Vec2f(centerX+v1, 0.0f), Vec2f(centerX+v1, windowHeight) );
    gl::drawLine( Vec2f(centerX-v1, 0.0f), Vec2f(centerX-v1, windowHeight) );
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
