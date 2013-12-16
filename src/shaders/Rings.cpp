//
//  Rings.cpp
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

#include "OculonApp.h"
#include "Interface.h"
#include "Rings.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include "cinder/Timeline.h"
#include <boost/format.hpp>

using namespace ci;
using namespace ci::app;

Rings::Rings()
: Scene("rings")
{
}

Rings::~Rings()
{
}

void Rings::setup()
{
    Scene::setup();
    
    mShader = loadFragShader("rings_frag.glsl");
    
    mAnimTime = 0.3f;
    
    mRingSetParams[0].mColor = ColorA(1.0f,1.0f,1.0f,1.0f);
    mRingSetParams[0].mTimeScale = -1.0f;
    mRingSetParams[0].mFrequency = 20.0f;
    mRingSetParams[0].mScale = 1.0f;
    mRingSetParams[0].mThickness = 0.15f;
    mRingSetParams[0].mPower = 0.5f;
    mRingSetParams[0].mCenter = Vec2f(0.5f,0.5f);
    mRingSetParams[0].mResponseBand = 0;
    
    mRingSetParams[1].mColor = ColorA(1.0f,1.0f,1.0f,0.0f);
    mRingSetParams[1].mTimeScale = 1.0f;
    mRingSetParams[1].mFrequency = 20.0f;
    mRingSetParams[1].mScale = 1.0f;
    mRingSetParams[1].mThickness = 0.15f;
    mRingSetParams[1].mPower = 0.5f;
    mRingSetParams[1].mCenter = Vec2f(0.5f,0.5f);
    mRingSetParams[1].mResponseBand = 1;
    
    mRingSetParams[2].mColor = ColorA(1.0f,1.0f,1.0f,0.0f);
    mRingSetParams[2].mTimeScale = 1.0f;
    mRingSetParams[2].mFrequency = 50.0f;
    mRingSetParams[2].mScale = 1.0f;
    mRingSetParams[2].mThickness = 0.15f;
    mRingSetParams[2].mPower = 0.5f;
    mRingSetParams[2].mCenter = Vec2f(0.5f,0.5f);
    mRingSetParams[2].mResponseBand = 2;
    
    mRingSetParams[3].mColor = ColorA(1.0f,1.0f,1.0f,0.0f);
    mRingSetParams[3].mTimeScale = 1.0f;
    mRingSetParams[3].mFrequency = 50.0f;
    mRingSetParams[3].mScale = 1.0f;
    mRingSetParams[3].mThickness = 0.15f;
    mRingSetParams[3].mPower = 0.5f;
    mRingSetParams[3].mCenter = Vec2f(0.5f,0.5f);
    mRingSetParams[3].mResponseBand = 2;
    
    for (int i = 0; i < NUM_RING_SETS; ++i)
    {
        mRingSetParams[i].mElapsedTime = 0.0f;
        mRingSetParams[i].mScaleByAudio = false;
        mRingSetParams[i].mPowerByAudio = false;
        mRingSetParams[i].mPrevFrequency = mRingSetParams[i].mFrequency;
        mRingSetParams[i].mPrevPower = mRingSetParams[i].mPrevPower;
        mRingSetParams[i].mAudioInputHandler.setup(false);
        mRingSetParams[i].mActualFrequency = mRingSetParams[i].mFrequency;
        mRingSetParams[i].mActualPower = mRingSetParams[i].mPower;
        mRingSetParams[i].mActualScale = mRingSetParams[i].mScale;
        mRingSetParams[i].mActualCenter = mRingSetParams[i].mCenter;
        
        mRingSetParams[i].mSeparateByAudio = false;
        mRingSetParams[i].mSpin = false;
        mRingSetParams[i].mSpinRate = 0.4f;
        mRingSetParams[i].mSpinTheta = 0.0f;
        mRingSetParams[i].mSpinRadius = 0.1f;
    }
    
    reset();
}

void Rings::reset()
{
    for (int i = 0; i < NUM_RING_SETS; ++i)
    {
        mRingSetParams[i].mElapsedTime = 0.0f;
    }
}

void Rings::setupInterface()
{
    mInterface->addParam(CreateFloatParam("animtime", &mAnimTime)
                         .minValue(0.01f)
                         .maxValue(10.0f));
    
    for (int i = 0; i < NUM_RING_SETS; ++i)
    {
        std::string indexStr = std::to_string(i+1);
        std::string name("ring" + indexStr);
        
        mInterface->gui()->addColumn();
        mInterface->gui()->addLabel(name);
        
        name = std::string(mName + "/ring" + indexStr);
        
        // all param names must be unique for load/save to work!
        if (i == 0) {
            mInterface->addButton(CreateTriggerParam("sync all", NULL))->registerCallback( boost::bind( &Rings::syncParams, this, 0, 0) );
            mInterface->addButton(CreateTriggerParam("sync <--", NULL))->registerCallback( boost::bind( &Rings::syncParams, this, 1, 0) );
        }
        if (i == 1) {
            mInterface->addButton(CreateTriggerParam("--> sync", NULL))->registerCallback( boost::bind( &Rings::syncParams, this, 0, 1) );
            mInterface->addButton(CreateTriggerParam("sync <--", NULL))->registerCallback( boost::bind( &Rings::syncParams, this, 2, 1) );
        }
        else if (i == 2) {
            mInterface->addButton(CreateTriggerParam("--> sync", NULL))->registerCallback( boost::bind( &Rings::syncParams, this, 1, 2) );
            mInterface->addButton(CreateTriggerParam("sync <--", NULL))->registerCallback( boost::bind( &Rings::syncParams, this, 3, 2) );
        }
        else if (i == 3) {
            mInterface->addButton(CreateTriggerParam("--> sync", NULL))->registerCallback( boost::bind( &Rings::syncParams, this, 2, 3) );
            mInterface->addButton(CreateTriggerParam("sync all", NULL))->registerCallback( boost::bind( &Rings::syncParams, this, 3, 3) );
        }
        
        mInterface->addParam(CreateFloatParam("timescale" + indexStr, &mRingSetParams[i].mTimeScale)
                             .minValue(-30.0f)
                             .maxValue(30.0f)
                             .oscReceiver(name));
        mInterface->addParam(CreateFloatParam("power" + indexStr, &mRingSetParams[i].mPower)
                             .minValue(0.1f)
                             .maxValue(1.5f)
                             .oscReceiver(name))->registerCallback( boost::bind( &Rings::updatePower, this, i) );
        
        mInterface->addParam(CreateFloatParam("frequency" + indexStr, &mRingSetParams[i].mFrequency)
                             .minValue(0.00001f)
                             .maxValue(200.0f)
                             .oscReceiver(name))->registerCallback( boost::bind( &Rings::updateFreq, this, i) );
        mInterface->addParam(CreateFloatParam("scale" + indexStr, &mRingSetParams[i].mScale)
                             .minValue(0.00001f)
                             .maxValue(1.0f)
                             .oscReceiver(name))->registerCallback( boost::bind( &Rings::updateScale, this, i) );

        mInterface->addParam(CreateFloatParam("thickness" + indexStr, &mRingSetParams[i].mThickness)
                             .minValue(0.01f)
                             .maxValue(1.0f)
                             .oscReceiver(name));
        mInterface->addParam(CreateVec2fParam("center" + indexStr, &mRingSetParams[i].mCenter, Vec2f::zero(), Vec2f::one())
                             .oscReceiver(name))->registerCallback( boost::bind( &Rings::updateCenter, this, i) );
        mInterface->addParam(CreateColorParam("color" + indexStr, &mRingSetParams[i].mColor(), kMinColor, kMaxColor));
        // color presets
        mInterface->addButton(CreateTriggerParam("off", NULL)
                              .oscReceiver(name))->registerCallback( boost::bind( &Rings::setRingColor, this, i, COLOR_NONE) );
        mInterface->addButton(CreateTriggerParam("white", NULL)
                              .oscReceiver(name))->registerCallback( boost::bind( &Rings::setRingColor, this, i, COLOR_WHITE) );
        mInterface->addButton(CreateTriggerParam("red", NULL)
                              .oscReceiver(name))->registerCallback( boost::bind( &Rings::setRingColor, this, i, COLOR_RED) );
        // animations
        mInterface->addButton(CreateTriggerParam("zoom in", NULL)
                              .oscReceiver(name))->registerCallback( boost::bind( &Rings::zoomIn, this, i) );
        mInterface->addButton(CreateTriggerParam("zoom out", NULL)
                              .oscReceiver(name))->registerCallback( boost::bind( &Rings::zoomOut, this, i) );
        mInterface->addButton(CreateTriggerParam("zoom restore", NULL)
                              .oscReceiver(name))->registerCallback( boost::bind( &Rings::zoomRestore, this, i) );
        
        // spin
        mInterface->addParam(CreateBoolParam("spin", &mRingSetParams[i].mSpin));
        mInterface->addParam(CreateBoolParam("sepaudio", &mRingSetParams[i].mSeparateByAudio));
        mInterface->addParam(CreateFloatParam("spinrate", &mRingSetParams[i].mSpinRate)
                             .minValue(-2.0f)
                             .maxValue(2.0f));
        mInterface->addParam(CreateFloatParam("spinsep", &mRingSetParams[i].mSpinRadius)
                             .minValue(0.0f)
                             .maxValue(1.0f));
        
        // audio
        mInterface->addParam(CreateBoolParam("power-audio" + indexStr, &mRingSetParams[i].mPowerByAudio)
                             .oscReceiver(name));
        mInterface->addParam(CreateBoolParam("scale-audio" + indexStr, &mRingSetParams[i].mScaleByAudio)
                             .oscReceiver(name));
        mInterface->addParam(CreateIntParam("audio-band" + indexStr, &mRingSetParams[i].mResponseBand)
                             .maxValue(2));
        
    }
    
    for (int i = 0; i < NUM_RING_SETS; ++i)
    {
        std::string indexStr = std::to_string(i+1);
        std::string name("ring" + indexStr);
        mRingSetParams[i].mAudioInputHandler.setupInterface(mInterface, name);
    }
}

#pragma mark - CALLBACKS

bool Rings::setRingColor(const int ringIndex, const int colorIndex)
{
    switch (colorIndex) {
        case COLOR_NONE:
            //mRingSetParams[ringIndex].mColor().a = 0.0f;
            timeline().apply( &mRingSetParams[ringIndex].mColor, ColorA(1.0f,1.0f,1.0f,0.0f), 1.0f,EaseOutExpo() );
            break;
            
        case COLOR_WHITE:
            timeline().apply( &mRingSetParams[ringIndex].mColor, ColorA::white(), 1.0f,EaseOutExpo() );
            break;
            
        case COLOR_RED:
            mRingSetParams[ringIndex].mColor = ColorA(1.0f,0.0f,0.0f,1.0f);
            break;
            
        default:
            break;
    }
    
    return true;
}

bool Rings::updateFreq(const int ringIndex)
{
    timeline().apply( &mRingSetParams[ringIndex].mActualFrequency, mRingSetParams[ringIndex].mFrequency, mAnimTime, EaseOutExpo() );
    return true;
}
bool Rings::updateScale(const int ringIndex)
{
    timeline().apply( &mRingSetParams[ringIndex].mActualScale, mRingSetParams[ringIndex].mScale, mAnimTime, EaseOutExpo() );
    return true;
}
bool Rings::updatePower(const int ringIndex)
{
    timeline().apply( &mRingSetParams[ringIndex].mActualPower, mRingSetParams[ringIndex].mPower, mAnimTime, EaseOutExpo() );
    return true;
}
bool Rings::updateCenter(const int ringIndex)
{
    timeline().apply( &mRingSetParams[ringIndex].mActualCenter, mRingSetParams[ringIndex].mCenter, mAnimTime, EaseOutExpo() );
    return true;
}

bool Rings::zoomIn(const int ringIndex)
{
    mRingSetParams[ringIndex].mPrevFrequency = mRingSetParams[ringIndex].mFrequency;
    mRingSetParams[ringIndex].mPrevPower = mRingSetParams[ringIndex].mPower;
    timeline().apply( &mRingSetParams[ringIndex].mActualFrequency, 200.0f, 1.0f, EaseInOutExpo() );
    timeline().apply( &mRingSetParams[ringIndex].mActualPower, 0.1f, 1.0f, EaseInOutExpo() );
    
    //ColorA toColor = mRingSetParams[ringIndex].mColor();
    //toColor.a = 1.0f;
    //timeline().apply( &mRingSetParams[ringIndex].mColor, toColor, 3.0f, EaseInQuad() );
    
    return true;
}

bool Rings::zoomOut(const int ringIndex)
{
    mRingSetParams[ringIndex].mPrevFrequency = mRingSetParams[ringIndex].mFrequency;
    mRingSetParams[ringIndex].mPrevPower = mRingSetParams[ringIndex].mPower;
    timeline().apply( &mRingSetParams[ringIndex].mActualFrequency, 2.0f, 1.0f, EaseInOutExpo() );
    timeline().apply( &mRingSetParams[ringIndex].mActualPower, 0.6f, 1.0f, EaseInOutExpo() );
    
    //ColorA toColor = mRingSetParams[ringIndex].mColor();
    //toColor.a = 0.0f;
    //timeline().apply( &mRingSetParams[ringIndex].mColor, toColor, 3.0f, EaseInQuad() );
    
    return true;
}

bool Rings::zoomRestore(const int ringIndex)
{
    timeline().apply( &mRingSetParams[ringIndex].mActualFrequency, mRingSetParams[ringIndex].mPrevFrequency, 1.0f, EaseOutExpo() );
    timeline().apply( &mRingSetParams[ringIndex].mActualPower, mRingSetParams[ringIndex].mPrevPower, 1.0f, EaseOutExpo() );
    
    return true;
}

bool Rings::syncParams( const int srcIndex, const int destIndex )
{
    if (destIndex == srcIndex)
    {
        for (int i = 1; i < NUM_RING_SETS; ++i)
        {
            mRingSetParams[i] = mRingSetParams[srcIndex];
        }
    }
    else
    {
        Vec2f center = mRingSetParams[destIndex].mCenter;
        mRingSetParams[destIndex] = mRingSetParams[srcIndex];
        // restore center
        mRingSetParams[destIndex].mCenter = center;
    }
    return true;
}

#pragma mark - UPDATE/DRAW

void Rings::update(double dt)
{
    Scene::update(dt);
    
    for (int i = 0; i < NUM_RING_SETS; ++i)
    {
        mRingSetParams[i].mAudioInputHandler.update(dt, mApp->getAudioInput(), mGain);
        mRingSetParams[i].mElapsedTime += mRingSetParams[i].mTimeScale*dt;
        
        if (mRingSetParams[i].mSpin) {
            float r = mRingSetParams[i].mSpinRadius;
            if (mRingSetParams[i].mSeparateByAudio)
            {
                float audioLevel = mRingSetParams[i].mAudioInputHandler.getAverageVolumeLowFreq();
                r *= audioLevel;
            }
            if (i == 1 || i == 3) r *= -1.0f;
            mRingSetParams[i].mSpinTheta += dt * mRingSetParams[i].mSpinRate;
            mRingSetParams[i].mActualCenter().x = mRingSetParams[0].mCenter.x + r * sin( mRingSetParams[i].mSpinTheta);
            mRingSetParams[i].mActualCenter().y = mRingSetParams[0].mCenter.y + r * cos( mRingSetParams[i].mSpinTheta);
//            mRingSetParams[1].mActualCenter().x = mRingSetParams[1].mCenter.x - r * sin(mSpinTheta*0.25f);
//            mRingSetParams[1].mActualCenter().y = mRingSetParams[1].mCenter.y - r * cos(mSpinTheta*0.25f);
        }
    }
    
    
}

void Rings::draw()
{
    gl::pushMatrices();

    drawScene();
    
    gl::popMatrices();
}

void Rings::shaderPreDraw()
{
    // audio texture
//    if( mAudioInputHandler.hasTexture() )
//    {
//        mAudioInputHandler.getFbo().bindTexture(1);
//    }
//
    float scale[NUM_RING_SETS];
    float power[NUM_RING_SETS];
    
    // when audio-reactive, the slider values are treated as the min
//    const float minPower = 0.1f;
//    const float minScale = 0.001f;
    for (int i = 0; i < NUM_RING_SETS; ++i)
    {
        float audioLevel = 1.0f;
        switch (mRingSetParams[i].mResponseBand) {
            case 0:
            audioLevel = mRingSetParams[i].mAudioInputHandler.getAverageVolumeLowFreq();
            break;
            case 1:
            audioLevel = mRingSetParams[i].mAudioInputHandler.getAverageVolumeMidFreq();
            break;
            case 2:
            audioLevel = mRingSetParams[i].mAudioInputHandler.getAverageVolumeHighFreq();
            break;
            default:
            break;
        }
        
        scale[i] = mRingSetParams[i].mActualScale;
        power[i] = mRingSetParams[i].mActualPower;
        
        if (mRingSetParams[i].mScaleByAudio)
        {
            scale[i] += audioLevel;
        }
        
        if (mRingSetParams[i].mPowerByAudio)
        {
            power[i] += audioLevel;
        }
    }
    
    mShader.bind();
    
    Vec3f resolution = Vec3f( mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f );
    
    mShader.uniform( "iResolution", resolution );
    //mShader.uniform( "iGlobalTime", (float)mElapsedTime );
    
    mShader.uniform( "iTime1", mRingSetParams[0].mElapsedTime);
    mShader.uniform( "iColor1", mRingSetParams[0].mColor);
    mShader.uniform( "iScale1", scale[0]);
    mShader.uniform( "iFrequency1", mRingSetParams[0].mActualFrequency);
    mShader.uniform( "iThickness1", mRingSetParams[0].mThickness);
    mShader.uniform( "iPower1", power[0]);
    mShader.uniform( "iCenter1", mRingSetParams[0].mActualCenter);
    
    mShader.uniform( "iTime2", mRingSetParams[1].mElapsedTime);
    mShader.uniform( "iColor2", mRingSetParams[1].mColor);
    mShader.uniform( "iScale2", scale[1]);
    mShader.uniform( "iFrequency2", mRingSetParams[1].mActualFrequency);
    mShader.uniform( "iThickness2", mRingSetParams[1].mThickness);
    mShader.uniform( "iPower2", power[1]);
    mShader.uniform( "iCenter2", mRingSetParams[1].mActualCenter);
    
    mShader.uniform( "iTime3", mRingSetParams[2].mElapsedTime);
    mShader.uniform( "iColor3", mRingSetParams[2].mColor);
    mShader.uniform( "iScale3", scale[2]);
    mShader.uniform( "iFrequency3", mRingSetParams[2].mActualFrequency);
    mShader.uniform( "iThickness3", mRingSetParams[2].mThickness);
    mShader.uniform( "iPower3", power[2]);
    mShader.uniform( "iCenter3", mRingSetParams[2].mActualCenter);
    
    mShader.uniform( "iTime4", mRingSetParams[3].mElapsedTime);
    mShader.uniform( "iColor4", mRingSetParams[3].mColor);
    mShader.uniform( "iScale4", scale[3]);
    mShader.uniform( "iFrequency4", mRingSetParams[3].mActualFrequency);
    mShader.uniform( "iThickness4", mRingSetParams[3].mThickness);
    mShader.uniform( "iPower4", power[3]);
    mShader.uniform( "iCenter4", mRingSetParams[3].mActualCenter);
}

void Rings::drawShaderOutput()
{
    // Draw shader output
    gl::enable( GL_TEXTURE_2D );
    gl::color( Colorf::white() );
    gl::begin( GL_TRIANGLES );
    
    // Define quad vertices
    const Area& bounds = mApp->getViewportBounds();
    
    Vec2f vert0( (float)bounds.x1, (float)bounds.y1 );
    Vec2f vert1( (float)bounds.x2, (float)bounds.y1 );
    Vec2f vert2( (float)bounds.x1, (float)bounds.y2 );
    Vec2f vert3( (float)bounds.x2, (float)bounds.y2 );
    
    // Define quad texture coordinates
    Vec2f uv0( 0.0f, 0.0f );
    Vec2f uv1( 1.0f, 0.0f );
    Vec2f uv2( 0.0f, 1.0f );
    Vec2f uv3( 1.0f, 1.0f );
    
    // Draw quad (two triangles)
    gl::texCoord( uv0 );
    gl::vertex( vert0 );
    gl::texCoord( uv2 );
    gl::vertex( vert2 );
    gl::texCoord( uv1 );
    gl::vertex( vert1 );
    
    gl::texCoord( uv1 );
    gl::vertex( vert1 );
    gl::texCoord( uv2 );
    gl::vertex( vert2 );
    gl::texCoord( uv3 );
    gl::vertex( vert3 );
    
    gl::end();
}

void Rings::shaderPostDraw()
{
    mShader.unbind();
    
    // audio texture
//    if( mAudioInputHandler.hasTexture() )
//    {
//        mAudioInputHandler.getFbo().unbindTexture();
//    }
}

void Rings::drawScene()
{
    gl::enableAlphaBlending();
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    gl::pushMatrices();
    gl::setMatricesWindow( mApp->getViewportSize() );
    
    shaderPreDraw();
    
    drawShaderOutput();
    
    shaderPostDraw();
    
    gl::popMatrices();
}

void Rings::drawDebug()
{
    //mAudioInputHandler.drawDebug(mApp->getViewportSize());
}
