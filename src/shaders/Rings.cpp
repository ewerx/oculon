//
//  Rings.cpp
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

#include "OculonApp.h"
#include "Rings.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include <boost/format.hpp>
#include "Interface.h"

using namespace ci;

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
    
    mAudioInputHandler.setup(false);
    
    mShader = loadFragShader("rings_frag.glsl");
    
    mRingSetParams[0].mColor = ColorA(1.0f,1.0f,1.0f,1.0f);
    mRingSetParams[0].mTimeScale = 1.0f;
    mRingSetParams[0].mElapsedTime = 0.0f;
    mRingSetParams[0].mZoom = 10.0f;
    mRingSetParams[0].mScale = 1.0f;
    mRingSetParams[0].mThickness = 0.25f;
    mRingSetParams[0].mPower = 0.5f;
    mRingSetParams[0].mCenter = Vec2f(0.5f,0.5f);
    mRingSetParams[0].mScaleByAudio = false;
    mRingSetParams[0].mPowerByAudio = false;
    
    mRingSetParams[1].mColor = ColorA(1.0f,1.0f,1.0f,1.0f);
    mRingSetParams[1].mTimeScale = 1.0f;
    mRingSetParams[1].mElapsedTime = 0.0f;
    mRingSetParams[1].mZoom = 10.0f;
    mRingSetParams[1].mScale = 1.0f;
    mRingSetParams[1].mThickness = 0.25f;
    mRingSetParams[1].mPower = 0.5f;
    mRingSetParams[1].mCenter = Vec2f(0.5f,0.5f);
    mRingSetParams[1].mScaleByAudio = false;
    mRingSetParams[1].mPowerByAudio = false;
    
    mRingSetParams[2].mColor = ColorA(1.0f,1.0f,1.0f,1.0f);
    mRingSetParams[2].mTimeScale = 1.0f;
    mRingSetParams[2].mElapsedTime = 0.0f;
    mRingSetParams[2].mZoom = 10.0f;
    mRingSetParams[2].mScale = 1.0f;
    mRingSetParams[2].mThickness = 0.25f;
    mRingSetParams[2].mPower = 0.5f;
    mRingSetParams[2].mCenter = Vec2f(0.5f,0.5f);
    mRingSetParams[2].mScaleByAudio = false;
    mRingSetParams[2].mPowerByAudio = false;
    
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
    for (int i = 0; i < NUM_RING_SETS; ++i)
    {
        char name[256];
        snprintf(name,256,"ring%d", i+1);
        
        mInterface->gui()->addColumn();
        mInterface->gui()->addLabel(name);
        
        snprintf(name,256,"%s/ring%d", mName.c_str(), i+1);
        
        mInterface->addParam(CreateFloatParam("timescale", &mRingSetParams[i].mTimeScale)
                             .minValue(-20.0f)
                             .maxValue(20.0f)
                             .oscReceiver(name));
        mInterface->addParam(CreateFloatParam("power", &mRingSetParams[i].mPower)
                             .minValue(0.1f)
                             .maxValue(1.5f)
                             .oscReceiver(name));
        mInterface->addParam(CreateBoolParam("power-audio-reactive", &mRingSetParams[i].mPowerByAudio)
                             .oscReceiver(name));
        mInterface->addParam(CreateFloatParam("frequency", &mRingSetParams[i].mZoom)
                             .minValue(0.00001f)
                             .maxValue(200.0f)
                             .oscReceiver(name));
        mInterface->addParam(CreateFloatParam("scale", &mRingSetParams[i].mScale)
                             .minValue(0.00001f)
                             .maxValue(1.0f)
                             .oscReceiver(name));
        mInterface->addParam(CreateBoolParam("scale-audio-reactive", &mRingSetParams[i].mScaleByAudio)
                             .oscReceiver(name));
        mInterface->addParam(CreateFloatParam("thickness", &mRingSetParams[i].mThickness)
                             .minValue(0.01f)
                             .maxValue(1.0f)
                             .oscReceiver(name));
        mInterface->addParam(CreateVec2fParam("center", &mRingSetParams[i].mCenter, Vec2f::zero(), Vec2f::one())
                             .oscReceiver(name));
        mInterface->addParam(CreateColorParam("color", &mRingSetParams[i].mColor, kMinColor, kMaxColor));
        // color presets
        mInterface->addButton(CreateTriggerParam("off", NULL)
                              .oscReceiver(name))->registerCallback( boost::bind( &Rings::setRingColor, this, i, COLOR_NONE) );
        mInterface->addButton(CreateTriggerParam("white", NULL)
                              .oscReceiver(name))->registerCallback( boost::bind( &Rings::setRingColor, this, i, COLOR_WHITE) );
        mInterface->addButton(CreateTriggerParam("red", NULL)
                              .oscReceiver(name))->registerCallback( boost::bind( &Rings::setRingColor, this, i, COLOR_RED) );
        
    }
    
    mAudioInputHandler.setupInterface(mInterface, mName);
}

#pragma mark - CALLBACKS

bool Rings::setRingColor(const int ringIndex, const int colorIndex)
{
    switch (colorIndex) {
        case COLOR_NONE:
            mRingSetParams[ringIndex].mColor.a = 0.0f;
            break;
            
        case COLOR_WHITE:
            mRingSetParams[ringIndex].mColor = ColorA::white();
            break;
            
        case COLOR_RED:
            mRingSetParams[ringIndex].mColor = ColorA(1.0f,0.0f,0.0f,1.0f);
            break;
            
        default:
            break;
    }
    
    return true;
}

#pragma mark - UPDATE/DRAW

void Rings::update(double dt)
{
    Scene::update(dt);
    
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    for (int i = 0; i < NUM_RING_SETS; ++i)
    {
        mRingSetParams[i].mElapsedTime += mRingSetParams[i].mTimeScale*dt;
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
    float audioLevel[NUM_RING_SETS];
    
    audioLevel[0] = mAudioInputHandler.getAverageVolumeLowFreq() * mGain;
    audioLevel[1] = mAudioInputHandler.getAverageVolumeMidFreq() * mGain;
    audioLevel[2] = mAudioInputHandler.getAverageVolumeHighFreq() * mGain;
    
    // when audio-reactive, the slider values are treated as the max
    const float minPower = 0.1f;
    const float minScale = 0.001f;
    for (int i = 0; i < NUM_RING_SETS; ++i)
    {
        scale[i] = mRingSetParams[i].mScale;
        power[i] = mRingSetParams[i].mPower;
        
        if (mRingSetParams[i].mScaleByAudio)
        {
            scale[i] = minScale + scale[i] * audioLevel[i];
        }
        
        if (mRingSetParams[i].mPowerByAudio)
        {
            power[i] = minPower + power[i] * audioLevel[i];
        }
    }
    
    mShader.bind();
    
    Vec3f resolution = Vec3f( mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f );
    
    mShader.uniform( "iResolution", resolution );
    //mShader.uniform( "iGlobalTime", (float)mElapsedTime );
    
    mShader.uniform( "iTime1", mRingSetParams[0].mElapsedTime);
    mShader.uniform( "iColor1", mRingSetParams[0].mColor);
    mShader.uniform( "iScale1", scale[0]);
    mShader.uniform( "iZoom1", mRingSetParams[0].mZoom);
    mShader.uniform( "iThickness1", mRingSetParams[0].mThickness);
    mShader.uniform( "iPower1", power[0]);
    mShader.uniform( "iCenter1", mRingSetParams[0].mCenter);
    
    mShader.uniform( "iTime2", mRingSetParams[1].mElapsedTime);
    mShader.uniform( "iColor2", mRingSetParams[1].mColor);
    mShader.uniform( "iScale2", scale[1]);
    mShader.uniform( "iZoom2", mRingSetParams[1].mZoom);
    mShader.uniform( "iThickness2", mRingSetParams[1].mThickness);
    mShader.uniform( "iPower2", power[1]);
    mShader.uniform( "iCenter2", mRingSetParams[1].mCenter);
    
    mShader.uniform( "iTime3", mRingSetParams[2].mElapsedTime);
    mShader.uniform( "iColor3", mRingSetParams[2].mColor);
    mShader.uniform( "iScale3", scale[2]);
    mShader.uniform( "iZoom3", mRingSetParams[2].mZoom);
    mShader.uniform( "iThickness3", mRingSetParams[2].mThickness);
    mShader.uniform( "iPower3", power[2]);
    mShader.uniform( "iCenter3", mRingSetParams[2].mCenter);
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
    mAudioInputHandler.drawDebug(mApp->getViewportSize());
}
