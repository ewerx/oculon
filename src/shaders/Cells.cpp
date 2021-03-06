//
//  Cells.cpp
//  Oculon
//
//  Created by Ehsan on 13-11-15.
//  Copyright 2013 ewerx. All rights reserved.
//

#include "Cells.h"
#include "Interface.h"
#include "OculonApp.h"
#include "Utils.h"
#include "cinder/Rand.h"
#include "cinder/Timeline.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace std;

Cells::Cells(const std::string& name)
: TextureShaders(name)
{
    mColor1 = ColorA::white();
    mColor2 = ColorA::black();
}

Cells::~Cells()
{
}

void Cells::setupShaders()
{
    mShaderType = 0;
    
    mShaders.push_back( new MultiLayer() );
    mShaders.push_back( new Worley() );
    mShaders.push_back( new GravityFieldShader() );
}

#pragma mark - Circle

Cells::MultiLayer::MultiLayer()
: FragShader("multilayer", "cells_frag.glsl")
{
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
    
    for (int i = 0; i < CELLS_NUM_LAYERS; ++i)
    {
        mTime[i] = 0.0f;
    }
}

void Cells::MultiLayer::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    interface->addParam(CreateFloatParam( "Zoom", &mZoom )
                         .minValue(0.01f)
                         .maxValue(3.0f)
                         .oscReceiver(getName()));
    
    interface->addParam(CreateFloatParam( "highlight", &mHighlight )
                         .maxValue(2.0f)
                         .oscReceiver(getName())
                         .midiInput(0, 1, 13));
    interface->addParam(CreateFloatParam( "intensity", &mIntensity )
                         .minValue(1.0f)
                         .maxValue(8.0f)
                         .oscReceiver(getName()));
    
    interface->gui()->addColumn();
    interface->addParam(CreateFloatParam( "TimeStep1", &mTimeStep[0] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName())
                         .midiInput(0, 1, 14));
    interface->addParam(CreateFloatParam( "TimeStep2", &mTimeStep[1] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName())
                         .midiInput(0, 1, 15));
    interface->addParam(CreateFloatParam( "TimeStep3", &mTimeStep[2] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam( "TimeStep4", &mTimeStep[3] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam( "TimeStep5", &mTimeStep[4] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam( "TimeStep6", &mTimeStep[5] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam( "TimeStep7", &mTimeStep[6] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    
    interface->gui()->addColumn();
    interface->addParam(CreateFloatParam( "Frequency1", &mFrequency[0] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam( "Frequency2", &mFrequency[1] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam( "Frequency3", &mFrequency[2] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam( "Frequency4", &mFrequency[3] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam( "Frequency5", &mFrequency[4] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam( "Frequency6", &mFrequency[5] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam( "Frequency7", &mFrequency[6] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    
    // audio params
    interface->gui()->addColumn();
    vector<string> audioResponseTypeNames;
#define AUDIO_RESPONSE_TYPE_ENTRY( nam, enm ) \
audioResponseTypeNames.push_back(nam);
    AUDIO_RESPONSE_TYPE_TUPLE
#undef  AUDIO_RESPONSE_TYPE_ENTRY
    interface->addEnum(CreateEnumParam( "AudioResponse", (int*)(&mAudioResponseType) )
                        .maxValue(AUDIO_RESPONSE_TYPE_COUNT)
                        .oscReceiver(getName())
                        .isVertical(), audioResponseTypeNames);
    
    interface->addParam(CreateFloatParam("freqmin", &mAudioResponseFreqMin)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam("freqmax", &mAudioResponseFreqMax)
                         .oscReceiver(getName()));
}

void Cells::MultiLayer::update(double dt)
{
    for (int i = 0; i < CELLS_NUM_LAYERS; ++i)
    {
        mTime[i] += dt * mTimeStep[i];
    }
}

void Cells::MultiLayer::setCustomParams( AudioInputHandler& audioInputHandler )
{
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
        float brightness = mHighlight * 100.0f;
        
        AudioInputHandler &aih = audioInputHandler;
        mShader.uniform("iBrightness1", brightness * aih.getAverageVolumeByFrequencyRange(0.0f, 0.1f) );
        mShader.uniform("iBrightness2", brightness * aih.getAverageVolumeByFrequencyRange(0.1f, 0.2f) );
        mShader.uniform("iBrightness3", brightness * aih.getAverageVolumeByFrequencyRange(0.2f, 0.35f));
        mShader.uniform("iBrightness4", brightness * aih.getAverageVolumeByFrequencyRange(0.35f, 0.5f));
        mShader.uniform("iBrightness5", brightness * aih.getAverageVolumeByFrequencyRange(0.5f, 0.65f));
        mShader.uniform("iBrightness6", brightness * aih.getAverageVolumeByFrequencyRange(0.65f, 0.8f));
        mShader.uniform("iBrightness7", brightness * aih.getAverageVolumeByFrequencyRange(0.8f, 1.0f) );
    }
    else
    {
        float brightness = mHighlight * 100.0f;
        
        if (mAudioResponseType == AUDIO_RESPONSE_SINGLE)
        {
            brightness *= audioInputHandler.getAverageVolumeByFrequencyRange(mAudioResponseFreqMin, mAudioResponseFreqMax);
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

#pragma mark - Worley

Cells::Worley::Worley()
: FragShader("worley", "bluecells.frag")
{
    
}

void Cells::Worley::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
}

void Cells::Worley::update(double dt)
{
    
}

void Cells::Worley::setCustomParams( AudioInputHandler& audioInputHandler )
{
}


#pragma mark - GravityField

Cells::GravityFieldShader::GravityFieldShader()
: FragShader("gravityfield", "gravityfield_frag.glsl")
{
    mPoints = 64;
    mMode = 0;
    mPhase = 0.000001f;
    mField = 0.1f;
    mSpan = (1.0f/1.5f);
}

void Cells::GravityFieldShader::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    
    interface->addParam(CreateIntParam( "points", &mPoints )
                        .minValue(1)
                        .maxValue(256));
    interface->addParam(CreateIntParam( "mode", &mMode )
                        .minValue(0)
                        .maxValue(2));
    interface->addParam(CreateFloatParam( "phase", &mPhase )
                        .minValue(0.0f)
                        .maxValue(1.0f));
    mPhaseBand.setupInterface(interface, "phase-band");
    
    interface->addParam(CreateFloatParam( "field", &mField )
                        .minValue(0.0f)
                        .maxValue(0.5f));
    mFieldBand.setupInterface(interface, "field-band");
    
    interface->addParam(CreateFloatParam( "span", &mSpan )
                        .minValue(0.0f)
                        .maxValue(1.0f));
    mSpanBand.setupInterface(interface, "span-band");
}

void Cells::GravityFieldShader::update(double dt)
{
}

void Cells::GravityFieldShader::setCustomParams( AudioInputHandler& audioInputHandler )
{
    const float phase = mPhase * audioInputHandler.getAverageVolumeByBand(mPhaseBand());
    const float field = mField * audioInputHandler.getAverageVolumeByBand(mFieldBand());
    const float span = mSpan * audioInputHandler.getAverageVolumeByBand(mSpanBand());
    
    mShader.uniform( "iMode", mMode );
    mShader.uniform( "iPoints", mPoints );
    mShader.uniform( "iPhase", phase );
    mShader.uniform( "iField", field );
    mShader.uniform( "iSpan", span );
}
