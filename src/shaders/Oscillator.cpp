//
//  Oscillator.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 1/14/2014.
//  Copyright 2014 ewerx. All rights reserved.
//

#include "Oscillator.h"
#include "Interface.h"
#include "OculonApp.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

Oscillator::Oscillator()
: Scene("oscillator")
{
}

Oscillator::~Oscillator()
{
}

#pragma mark - Setup

void Oscillator::setup()
{
    Scene::setup();
    
    mShader = loadFragShader("oscillator_frag.glsl");
    
    // params
    for (int i = 0; i < MAX_WAVES; ++i)
    {
        mWaveParams[i].mElapsedTime = 0.0f;
        mWaveParams[i].mTimeScale = 1.0f;
        mWaveParams[i].mOffset = 0.5f;
    }
}

void Oscillator::reset()
{
    for (int i = 0; i < MAX_WAVES; ++i)
    {
        mWaveParams[i].mElapsedTime = 0.0f;
    }
}

void Oscillator::setupInterface()
{
    
}

#pragma mark - Update

void Oscillator::update(double dt)
{
    Scene::update(dt);
    
    for (int i = 0; i < MAX_WAVES; ++i)
    {
//        mWaveParams[i].mAudioInputHandler.update(dt, mApp->getAudioInput(), mGain);
        mWaveParams[i].mElapsedTime += mWaveParams[i].mTimeScale*dt;
    }
}

#pragma mark - Draw

void Oscillator::draw()
{
    glPushAttrib( GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT );
    gl::pushMatrices();
    
    gl::enableAlphaBlending();
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    gl::setMatricesWindow( mApp->getViewportSize() );
    
    shaderPreDraw();
    
    drawShaderOutput();
    
    shaderPostDraw();
    
    gl::popMatrices();
    glPopAttrib();
}

void Oscillator::shaderPreDraw()
{
    // audio texture
    //    if( mAudioInputHandler.hasTexture() )
    //    {
    //        mAudioInputHandler.getFbo().bindTexture(1);
    //    }

    
    mShader.bind();
    
    Vec3f resolution = Vec3f( mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f );
    
    mShader.uniform( "iResolution", resolution );
    
    mShader.uniform( "iGlobalTime", mWaveParams[0].mElapsedTime);
    mShader.uniform( "iOffset", mWaveParams[0].mOffset * mApp->getViewportHeight());
    mShader.uniform( "iPhase", 0.5f);
    mShader.uniform( "iAmplitude", 0.5f);
//    mShader.uniform( "iColor1", mWaveParams[0].mColor);
//    mShader.uniform( "iScale1", scale[0]);
//    mShader.uniform( "iFrequency1", mWaveParams[0].mActualFrequency);
//    mShader.uniform( "iThickness1", mWaveParams[0].mThickness);
//    mShader.uniform( "iPower1", power[0]);
//    mShader.uniform( "iCenter1", mWaveParams[0].mActualCenter);
//    mShader.uniform( "iFormat1", mWaveParams[0].mFormat);
    
//    mShader.uniform( "iTime2", mWaveParams[1].mElapsedTime);
//    mShader.uniform( "iColor2", mWaveParams[1].mColor);
//    mShader.uniform( "iScale2", scale[1]);
//    mShader.uniform( "iFrequency2", mWaveParams[1].mActualFrequency);
//    mShader.uniform( "iThickness2", mWaveParams[1].mThickness);
//    mShader.uniform( "iPower2", power[1]);
//    mShader.uniform( "iCenter2", mWaveParams[1].mActualCenter);
//    mShader.uniform( "iFormat2", mWaveParams[1].mFormat);
//    
//    mShader.uniform( "iTime3", mWaveParams[2].mElapsedTime);
//    mShader.uniform( "iColor3", mWaveParams[2].mColor);
//    mShader.uniform( "iScale3", scale[2]);
//    mShader.uniform( "iFrequency3", mWaveParams[2].mActualFrequency);
//    mShader.uniform( "iThickness3", mWaveParams[2].mThickness);
//    mShader.uniform( "iPower3", power[2]);
//    mShader.uniform( "iCenter3", mWaveParams[2].mActualCenter);
//    mShader.uniform( "iFormat3", mWaveParams[2].mFormat);
//    
//    mShader.uniform( "iTime4", mWaveParams[3].mElapsedTime);
//    mShader.uniform( "iColor4", mWaveParams[3].mColor);
//    mShader.uniform( "iScale4", scale[3]);
//    mShader.uniform( "iFrequency4", mWaveParams[3].mActualFrequency);
//    mShader.uniform( "iThickness4", mWaveParams[3].mThickness);
//    mShader.uniform( "iPower4", power[3]);
//    mShader.uniform( "iCenter4", mWaveParams[3].mActualCenter);
//    mShader.uniform( "iFormat4", mWaveParams[3].mFormat);
}

void Oscillator::drawShaderOutput()
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

void Oscillator::shaderPostDraw()
{
    mShader.unbind();
    
    // audio texture
    //    if( mAudioInputHandler.hasTexture() )
    //    {
    //        mAudioInputHandler.getFbo().unbindTexture();
    //    }
}

void Oscillator::drawDebug()
{
    Scene::drawDebug();
}
