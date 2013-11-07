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
    
    mShader = loadFragShader("rings_frag.glsl");
    
    mRingSetParams[0].mColor = ColorA(1.0f,1.0f,1.0f,1.0f);
    mRingSetParams[0].mTimeScale = 1.0f;
    mRingSetParams[0].mElapsedTime = 0.0f;
    mRingSetParams[0].mZoom = 5.0f;
    mRingSetParams[0].mScale = 1.0f;
    mRingSetParams[0].mThickness = 0.5f;
    mRingSetParams[0].mPower = 0.5f;
    
    mRingSetParams[1].mColor = ColorA(1.0f,1.0f,1.0f,1.0f);
    mRingSetParams[1].mTimeScale = 1.0f;
    mRingSetParams[1].mElapsedTime = 0.0f;
    mRingSetParams[1].mZoom = 5.0f;
    mRingSetParams[1].mScale = 1.0f;
    mRingSetParams[1].mThickness = 0.5f;
    mRingSetParams[1].mPower = 0.5f;
    
    mRingSetParams[2].mColor = ColorA(1.0f,1.0f,1.0f,1.0f);
    mRingSetParams[2].mTimeScale = 1.0f;
    mRingSetParams[2].mElapsedTime = 0.0f;
    mRingSetParams[2].mZoom = 1.0f;
    mRingSetParams[2].mScale = 5.0f;
    mRingSetParams[2].mThickness = 0.5f;
    mRingSetParams[2].mPower = 0.5f;
    
    mZoomByAudio = false;
    mPowerByAudio = false;
    
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
    mInterface->addParam(CreateBoolParam("audiozoom", &mZoomByAudio)
                         .oscReceiver(mName));
    mInterface->addParam(CreateBoolParam("audiopower", &mPowerByAudio)
                         .oscReceiver(mName));
    
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
        mInterface->addParam(CreateFloatParam("zoom", &mRingSetParams[i].mZoom)
                             .minValue(0.00001f)
                             .maxValue(200.0f)
                             .oscReceiver(name));
        mInterface->addParam(CreateFloatParam("scale", &mRingSetParams[i].mScale)
                             .minValue(0.0001f)
                             .maxValue(10.0f)
                             .oscReceiver(name));
        mInterface->addParam(CreateFloatParam("thickness", &mRingSetParams[i].mThickness)
                             .minValue(0.001f)
                             .maxValue(3.0f)
                             .oscReceiver(name));
        mInterface->addParam(CreateColorParam("color", &mRingSetParams[i].mColor, kMinColor, kMaxColor));
    }
    
    mApp->getAudioInputHandler().setupInterface(mInterface, mName);
}

void Rings::update(double dt)
{
    Scene::update(dt);
    
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
//    if( mApp->getAudioInputHandler().hasTexture() )
//    {
//        mApp->getAudioInputHandler().getFbo().bindTexture(1);
//    }
//    
    float zoom1 = mRingSetParams[0].mZoom;
    float zoom2 = mRingSetParams[1].mZoom;
    float zoom3 = mRingSetParams[2].mZoom;
    
    float power1 = mRingSetParams[0].mZoom;
    float power2 = mRingSetParams[1].mZoom;
    float power3 = mRingSetParams[2].mZoom;
    
    float fftlow = mGain * mApp->getAudioInputHandler().getAverageVolumeByFrequencyRange(0.0f, 0.333f);
    float fftmid = mGain * mApp->getAudioInputHandler().getAverageVolumeByFrequencyRange(0.333f, 0.666f);
    float ffthigh = mGain * mApp->getAudioInputHandler().getAverageVolumeByFrequencyRange(0.333f, 1.0f);
    
    if (mZoomByAudio)
    {
        float minZoom = 2.0f;
        zoom1 *= fftlow + minZoom;
        zoom2 *= fftmid + minZoom;
        zoom3 *= ffthigh + minZoom;
    }
    
    if (mPowerByAudio)
    {
        float minPower = 0.01f;
        power1 *= fftlow + minPower;
        power2 *= fftmid + minPower;
        power3 *= ffthigh + minPower;
    }
    
    mShader.bind();
    
    Vec3f resolution = Vec3f( mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f );
    
    mShader.uniform( "iResolution", resolution );
    //mShader.uniform( "iGlobalTime", (float)mElapsedTime );
    
    mShader.uniform( "iTime1", mRingSetParams[0].mElapsedTime);
    mShader.uniform( "iColor1", mRingSetParams[0].mColor);
    mShader.uniform( "iScale1", mRingSetParams[0].mScale);
    mShader.uniform( "iZoom1", zoom1);
    mShader.uniform( "iThickness1", mRingSetParams[0].mThickness);
    mShader.uniform( "iPower1", power1);
    
    mShader.uniform( "iTime2", mRingSetParams[1].mElapsedTime);
    mShader.uniform( "iColor2", mRingSetParams[1].mColor);
    mShader.uniform( "iScale2", mRingSetParams[1].mScale);
    mShader.uniform( "iZoom2", zoom2);
    mShader.uniform( "iThickness2", mRingSetParams[1].mThickness);
    mShader.uniform( "iPower2", power2);
    
    mShader.uniform( "iTime3", mRingSetParams[2].mElapsedTime);
    mShader.uniform( "iColor3", mRingSetParams[2].mColor);
    mShader.uniform( "iScale3", mRingSetParams[2].mScale);
    mShader.uniform( "iZoom3", zoom3);
    mShader.uniform( "iThickness3", mRingSetParams[2].mThickness);
    mShader.uniform( "iPower3", power3);
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
    if( mApp->getAudioInputHandler().hasTexture() )
    {
        mApp->getAudioInputHandler().getFbo().unbindTexture();
    }
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
    mApp->getAudioInputHandler().drawDebug(mApp->getViewportSize());
}
