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
    //mAudioInputHandler.setup(this, true);
}

Rings::~Rings()
{
}

void Rings::setup()
{
    Scene::setup();
    
    mShader = loadFragShader("rings_frag.glsl");
    
    mColor1 = ColorA(0.2f,0.0f,0.1f,1.0f);
    mColor2 = ColorA(0.9f,1.0f,1.0f,1.0f);
    mTimeScale = 1.0f;
    mNumRings = 64;
    mSmoothing = 0.003f;
    mIntervals = 2.0f;
    
    reset();
}

void Rings::reset()
{
    mElapsedTime = 0.0f;
}

void Rings::setupInterface()
{
    mInterface->addParam(CreateFloatParam( "TimeScale", &mTimeScale )
                         .minValue(-32.0f)
                         .maxValue(32.0f));
    mInterface->addParam(CreateFloatParam( "Smoothing", &mSmoothing )
                         .minValue(0.0f)
                         .maxValue(0.1f));
    mInterface->addParam(CreateIntParam( "NumRings", &mNumRings )
                         .minValue(1)
                         .maxValue(256));
    mInterface->addParam(CreateIntParam( "Intervals", &mIntervals )
                         .minValue(0)
                         .maxValue(128));
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor));
    
    //mAudioInputHandler.setupInterface(mInterface);
}

void Rings::update(double dt)
{
    Scene::update(dt);
    
    //mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    mElapsedTime += dt;
}

void Rings::draw()
{
    gl::pushMatrices();

    drawScene();
    
    gl::popMatrices();
}

void Rings::shaderPreDraw()
{
    mShader.bind();
    
    Vec3f resolution = Vec3f( mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f );
    
    mShader.uniform( "iResolution", resolution );
    mShader.uniform( "iGlobalTime", (float)mElapsedTime );
    mShader.uniform( "iColor1", mColor1);
    mShader.uniform( "iColor2", mColor2);
    
    mShader.uniform( "iTimeScale", mTimeScale );
    mShader.uniform( "iRings", (float)mNumRings );
    mShader.uniform( "iSmoothing", mSmoothing );
    mShader.uniform( "iIntervals", (float)mIntervals );
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
