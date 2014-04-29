//
//  Cells.cpp
//  Oculon
//
//  Created by Ehsan on 13-11-15.
//  Copyright 2013 ewerx. All rights reserved.
//

#include "OculonApp.h"
#include "Interface.h"
#include "Cells.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include "cinder/Timeline.h"
#include <boost/format.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;

Cells::Cells()
: Scene("cells")
{
}

Cells::~Cells()
{
}

void Cells::setup()
{
    Scene::setup();
    
    mAudioInputHandler.setup(false);
    
    mShader = loadFragShader("cells_frag.glsl");
    
    mColor1 = ColorA::white();
    mColor2 = ColorA::black();
    
    mAudioResponseFreqMin = 0.0f;
    mAudioResponseFreqMax = 1.0f;
    
    mAudioResponseType = AUDIO_RESPONSE_SINGLE;
    mZoom = 1.0f;
    mHighlight = 0.6f;
    mIntensity = 1.0f;
    
    mTimeStep[0] = 1.0f;
    mTimeStep[1] = 0.5f;
    mTimeStep[2] = 0.25f;
    mTimeStep[3] = 0.125f;
    mTimeStep[4] = 0.125f;
    mTimeStep[5] = 0.065f;
    mTimeStep[6] = 0.0f;
    
    mFrequency[0] = 1.0f;
    mFrequency[1] = 2.0f;
    mFrequency[2] = 4.0f;
    mFrequency[3] = 8.0f;
    mFrequency[4] = 32.0f;
    mFrequency[5] = 64.0f;
    mFrequency[6] = 128.0f;
    
    reset();
}

void Cells::reset()
{
    for (int i = 0; i < CELLS_NUM_LAYERS; ++i)
    {
        mTime[i] = 0.0f;
    }
}

void Cells::setupInterface()
{
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateFloatParam( "Zoom", &mZoom )
                         .minValue(0.01f)
                         .maxValue(3.0f)
                         .oscReceiver(getName()));
    
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor)
                         .oscReceiver(mName));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor)
                         .oscReceiver(mName));
    
    mInterface->addParam(CreateFloatParam( "highlight", &mHighlight )
                         .maxValue(2.0f)
                         .oscReceiver(getName())
                         .midiInput(1, 2, 17));
    mInterface->addParam(CreateFloatParam( "intensity", &mIntensity )
                         .minValue(1.0f)
                         .maxValue(8.0f)
                         .oscReceiver(getName()));
    
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateFloatParam( "TimeStep1", &mTimeStep[0] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName())
                         .midiInput(1, 2, 18));
    mInterface->addParam(CreateFloatParam( "TimeStep2", &mTimeStep[1] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName())
                         .midiInput(1, 2, 19));
    mInterface->addParam(CreateFloatParam( "TimeStep3", &mTimeStep[2] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "TimeStep4", &mTimeStep[3] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "TimeStep5", &mTimeStep[4] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "TimeStep6", &mTimeStep[5] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "TimeStep7", &mTimeStep[6] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateFloatParam( "Frequency1", &mFrequency[0] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "Frequency2", &mFrequency[1] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "Frequency3", &mFrequency[2] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "Frequency4", &mFrequency[3] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "Frequency5", &mFrequency[4] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "Frequency6", &mFrequency[5] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "Frequency7", &mFrequency[6] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    
    // audio params
    mInterface->gui()->addColumn();
    vector<string> audioResponseTypeNames;
#define AUDIO_RESPONSE_TYPE_ENTRY( nam, enm ) \
audioResponseTypeNames.push_back(nam);
    AUDIO_RESPONSE_TYPE_TUPLE
#undef  AUDIO_RESPONSE_TYPE_ENTRY
    mInterface->addEnum(CreateEnumParam( "AudioResponse", (int*)(&mAudioResponseType) )
                        .maxValue(AUDIO_RESPONSE_TYPE_COUNT)
                        .oscReceiver(getName())
                        .isVertical(), audioResponseTypeNames);
    
    mInterface->addParam(CreateFloatParam("freqmin", &mAudioResponseFreqMin)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("freqmax", &mAudioResponseFreqMax)
                         .oscReceiver(getName()));
    
    mAudioInputHandler.setupInterface(mInterface, mName);
}

#pragma mark - CALLBACKS

#pragma mark - UPDATE/DRAW

void Cells::update(double dt)
{
    Scene::update(dt);
    
    mAudioInputHandler.update(dt, mApp->getAudioInput(), mGain);
    
    for (int i = 0; i < CELLS_NUM_LAYERS; ++i)
    {
        mTime[i] += dt * mTimeStep[i];
    }
}

void Cells::draw()
{
    gl::pushMatrices();

    drawScene();
    
    gl::popMatrices();
}

void Cells::shaderPreDraw()
{
    // audio texture
//    if( mAudioInputHandler.hasTexture() )
//    {
//        mAudioInputHandler.getFbo().bindTexture(1);
//    }
//
    
    mShader.bind();
    
    Vec3f resolution = Vec3f( mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f );
    
    mShader.uniform( "iResolution", resolution );
    mShader.uniform( "iColor1", mColor1);
    mShader.uniform( "iColor2", mColor2);
    
    mShader.uniform("iZoom", mZoom);
    mShader.uniform("iHighlight", mHighlight);
    
    mShader.uniform("iTimeStep1", mTime[0]);
    mShader.uniform("iTimeStep2", mTime[1]);
    mShader.uniform("iTimeStep3", mTime[2]);
    mShader.uniform("iTimeStep4", mTime[3]);
    mShader.uniform("iTimeStep5", mTime[4]);
    mShader.uniform("iTimeStep6", mTime[5]);
    mShader.uniform("iTimeStep7", mTime[6]);
    
    mShader.uniform("iFrequency1", mFrequency[0]);
    mShader.uniform("iFrequency2", mFrequency[1]);
    mShader.uniform("iFrequency3", mFrequency[2]);
    mShader.uniform("iFrequency4", mFrequency[3]);
    mShader.uniform("iFrequency5", mFrequency[4]);
    mShader.uniform("iFrequency6", mFrequency[5]);
    mShader.uniform("iFrequency7", mFrequency[6]);
    mShader.uniform("iIntensity", mIntensity);
    
    if (mAudioResponseType == AUDIO_RESPONSE_MULTI)
    {
        AudioInputHandler &aih = mAudioInputHandler;
        mShader.uniform("iBrightness1", mHighlight * aih.getAverageVolumeByFrequencyRange(0.0f, 0.1f) * mGain * 10.0f);
        mShader.uniform("iBrightness2", mHighlight * aih.getAverageVolumeByFrequencyRange(0.1f, 0.2f) * mGain * 10.0f);
        mShader.uniform("iBrightness3", mHighlight * aih.getAverageVolumeByFrequencyRange(0.2f, 0.3f) * mGain * 10.0f);
        mShader.uniform("iBrightness4", mHighlight * aih.getAverageVolumeByFrequencyRange(0.3f, 0.4f) * mGain * 10.0f);
        mShader.uniform("iBrightness5", mHighlight * aih.getAverageVolumeByFrequencyRange(0.4f, 0.5f) * mGain * 10.0f);
        mShader.uniform("iBrightness6", mHighlight * aih.getAverageVolumeByFrequencyRange(0.6f, 0.7f) * mGain * 10.0f);
        mShader.uniform("iBrightness7", mHighlight * aih.getAverageVolumeByFrequencyRange(0.8f, 1.0f) * mGain * 10.0f);
    }
    else
    {
        float brightness = mHighlight;
        
        if (mAudioResponseType == AUDIO_RESPONSE_SINGLE)
        {
            brightness *= 100.0f * mAudioInputHandler.getAverageVolumeByFrequencyRange(mAudioResponseFreqMin, mAudioResponseFreqMax) * mGain;
        }
        
        mShader.uniform("iBrightness1", brightness);
        mShader.uniform("iBrightness2", brightness);
        mShader.uniform("iBrightness3", brightness);
        mShader.uniform("iBrightness4", brightness);
        mShader.uniform("iBrightness5", brightness);
        mShader.uniform("iBrightness6", brightness);
        mShader.uniform("iBrightness7", brightness);
    }
}

void Cells::drawShaderOutput()
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

void Cells::shaderPostDraw()
{
    mShader.unbind();
    
    // audio texture
//    if( mAudioInputHandler.hasTexture() )
//    {
//        mAudioInputHandler.getFbo().unbindTexture();
//    }
}

void Cells::drawScene()
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

void Cells::drawDebug()
{
    mAudioInputHandler.drawDebug(mApp->getViewportSize());
}
