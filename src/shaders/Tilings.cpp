//
//  Tilings.cpp
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

#include "OculonApp.h"
#include "Tilings.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include <boost/format.hpp>
#include "Interface.h"

using namespace ci;

Tilings::Tilings()
: Scene("tilings")
{
}

Tilings::~Tilings()
{
}

void Tilings::setup()
{
    Scene::setup();
    
    mAudioInputHandler.setup(false);
    
    mShader = loadFragShader("tilings_frag.glsl");
    
    reset();
}

void Tilings::reset()
{
    mElapsedTime = 0.0f;
    
    mColor1 = ColorA::white();
    mColor2 = ColorA::black();
    mTimeScale = 0.02f;
    mOffset = 0.0f;
    
    mIterations = 20;
    mAngleP = 3;
    mAngleQ = 5;
    mAngleR = 2;
    mThickness = 0.03f;
    
    mAudioOffset = false;
    mResponseBand = 1;
}

void Tilings::setupInterface()
{
    mInterface->addParam(CreateFloatParam( "TimeScale", &mTimeScale )
                         .minValue(0.0f)
                         .maxValue(2.0f));
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor));
    
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateIntParam( "iterations", &mIterations )
                         .minValue(1)
                         .maxValue(40)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateIntParam( "anglep", &mAngleP )
                         .minValue(2)
                         .maxValue(40)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateIntParam( "angleq", &mAngleQ )
                         .minValue(2)
                         .maxValue(12)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateIntParam( "angler", &mAngleR )
                         .minValue(1)
                         .maxValue(12)
                         .oscReceiver(getName()));
    //mInterface->addParam(CreateVec3fParam("center", &mCenter, Vec3f::zero(), Vec3f::one()));
    mInterface->addParam(CreateFloatParam( "Thickness", &mThickness )
                         .minValue(0.0f)
                         .maxValue(1.0f));
    
    mInterface->addParam(CreateBoolParam("audio", &mAudioOffset));
    mInterface->addParam(CreateIntParam( "audioband", &mResponseBand )
                         .maxValue(2));
    
    mAudioInputHandler.setupInterface(mInterface, mName);
}

void Tilings::update(double dt)
{
    Scene::update(dt);
    
    mAudioInputHandler.update(dt, mApp->getAudioInput(), mGain);
    
    if (mAudioOffset) {
        float audioLevel = 0.0f;
        switch (mResponseBand) {
            case 0:
            audioLevel = mAudioInputHandler.getAverageVolumeLowFreq();
            break;
            case 1:
            audioLevel = mAudioInputHandler.getAverageVolumeMidFreq();
            break;
            case 2:
            audioLevel = mAudioInputHandler.getAverageVolumeHighFreq();
            break;
            default:
            break;
        }
        mOffset = audioLevel * 5.0f;
    }
    
    
    mElapsedTime += dt * mTimeScale;
}

void Tilings::draw()
{
    gl::pushMatrices();

    drawScene();
    
    gl::popMatrices();
}

void Tilings::shaderPreDraw()
{
    mShader.bind();
    
    Vec3f resolution = Vec3f( mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f );
    
    mShader.uniform( "iResolution", resolution );
    mShader.uniform( "iGlobalTime", (float)mElapsedTime );
    
    mShader.uniform( "iTimeScale", mTimeScale );
    mShader.uniform( "iColor1", mColor1 );
    mShader.uniform( "iColor2", mColor2 );
    mShader.uniform( "iIterations", mIterations );
    mShader.uniform( "iAngleP", mAngleP );
    mShader.uniform( "iAngleQ", mAngleQ );
    mShader.uniform( "iAngleR", mAngleR );
    mShader.uniform( "iCenter", mCenter );
    mShader.uniform( "iThickness", mThickness );
    mShader.uniform( "iOffset", mOffset);
}

void Tilings::drawShaderOutput()
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

void Tilings::shaderPostDraw()
{
    mShader.unbind();
}

void Tilings::drawScene()
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

void Tilings::drawDebug()
{
    //mAudioInputHandler.drawDebug(mApp->getViewportSize());
}
