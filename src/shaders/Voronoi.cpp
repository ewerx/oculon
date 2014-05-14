//
//  Voronoi.cpp
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

#include "OculonApp.h"
#include "Voronoi.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include <boost/format.hpp>
#include "Interface.h"

using namespace ci;

Voronoi::Voronoi()
: Scene("voronoi")
{
}

Voronoi::~Voronoi()
{
}

void Voronoi::setup()
{
    Scene::setup();
    
    mAudioInputHandler.setup(false);
    
    mShader = loadFragShader("voronoi_frag.glsl");
    
    reset();
}

void Voronoi::reset()
{
    mElapsedTime = 0.0f;
    
    mColor1 = ColorA::white();
    mColor2 = ColorA::black();
    mTimeScale = 0.02f;
    
    mBorderColor = Vec3f( 1.0f, 1.0f, 1.0f );
    mZoom = 60.0f;
    mBorderIn = 0.0f;
    mBorderOut = 0.075f;
    mSeedColor = Vec3f( 1.0f, 1.0f, 1.0f );
    mSeedSize = 0.0f;
    mCellLayers = 8.0f;
    mCellBrightness = 0.5f;
    mCellBorderStrength = 0.5f;
    mCellColor = Vec3f( 0.0f, 0.0f, 0.0f );
    mSpeed = 1.00f;
    mDistortion = 0.0f;
    mAudioDistortion = true;
    
    mAudioOffset = false;
    mResponseBand = 1;
}

void Voronoi::setupInterface()
{
    mInterface->addParam(CreateFloatParam( "TimeScale", &mTimeScale )
                         .minValue(0.0f)
                         .maxValue(2.0f));
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor));
    
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("Voronoi");
    mInterface->addParam(CreateFloatParam("voronoi/speed", &mSpeed)
                         .maxValue(10.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("voronoi/zoom", &mZoom)
                         .maxValue(256.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("voronoi/distortion", &mDistortion)
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    mInterface->gui()->addSeparator();
    mInterface->addParam(CreateVec3fParam("voronoi/line_color", &mBorderColor, Vec3f::zero(), Vec3f(3.0f,3.0f,3.0f)));
    mInterface->addParam(CreateFloatParam("voronoi/borderin", &mBorderIn)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("voronoi/borderout", &mBorderOut)
                         .oscReceiver(getName()));
    mInterface->gui()->addSeparator();
    mInterface->addParam(CreateVec3fParam("voronoi/seed_color", &mSeedColor, Vec3f::zero(), Vec3f(3.0f,3.0f,3.0f)));
    mInterface->addParam(CreateFloatParam("voronoi/seedsize", &mSeedSize)
                         .oscReceiver(getName()));
    mInterface->gui()->addSeparator();
    mInterface->addParam(CreateVec3fParam("voronoi/cell_color", &mCellColor, Vec3f::zero(), Vec3f(3.0f,3.0f,3.0f)));
    mInterface->addParam(CreateFloatParam("voronoi/cell_brightness", &mCellBrightness)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("voronoi/cell_strength", &mCellBorderStrength)
                         .oscReceiver(getName()));
    
    mInterface->addParam(CreateBoolParam("voronoi/audio_distortion", &mAudioDistortion)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateIntParam( "audioband", &mResponseBand )
                         .maxValue(2));
    
    mAudioInputHandler.setupInterface(mInterface, mName);
}

void Voronoi::update(double dt)
{
    Scene::update(dt);
    
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    mElapsedTime += dt * mTimeScale;
}

void Voronoi::draw()
{
    gl::pushMatrices();

    drawScene();
    
    gl::popMatrices();
}

void Voronoi::shaderPreDraw()
{
    mShader.bind();
    
    Vec3f resolution = Vec3f( mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f );
    
    mShader.uniform( "iResolution", resolution );
    mShader.uniform( "iGlobalTime", (float)mElapsedTime );
    mShader.uniform( "iColor1", mColor1);
    mShader.uniform( "iColor2", mColor2);
    
    mShader.uniform( "iResolution", resolution );
    mShader.uniform( "borderColor", mBorderColor );
    mShader.uniform( "zoom", mZoom );
    mShader.uniform( "speed", mSpeed );
    mShader.uniform( "borderIn", mBorderIn );
    mShader.uniform( "borderOut", mBorderOut );
    mShader.uniform( "seedSize", mSeedSize );
    mShader.uniform( "seedColor", mSeedColor );
    mShader.uniform( "cellLayers", mCellLayers );
    mShader.uniform( "cellColor", mCellColor );
    mShader.uniform( "cellBorderStrength", mCellBorderStrength );
    mShader.uniform( "cellBrightness", mCellBrightness );
    
    float distortion = mDistortion;
    if (mAudioDistortion)
    {
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
        distortion += 10.0f * audioLevel;
        distortion = math<float>::min(distortion, 1.0f);
    }
    mShader.uniform( "distortion", distortion );
}

void Voronoi::drawShaderOutput()
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

void Voronoi::shaderPostDraw()
{
    mShader.unbind();
}

void Voronoi::drawScene()
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

void Voronoi::drawDebug()
{
    //mAudioInputHandler.drawDebug(mApp->getViewportSize());
}
