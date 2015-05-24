//
//  Waves.cpp
//  Oculon1
//
//  Created by Ehsan Rezaie on 2015-05-05.
//
//

#include "OculonApp.h"
#include "Waves.h"

#include "Interface.h"
#include "Utils.h"

#include "cinder/Utilities.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

Waves::Waves()
: TextureShaders("waves")
{
}

Waves::~Waves()
{
}

void Waves::setupShaders()
{
    mShaderType = 0;
    
    mShaders.push_back( new MultiWave() );
    mShaders.push_back( new SineWave() );
    mShaders.push_back( new Oscilloscope() );
    mShaders.push_back( new AudioGraph() );
//    mShaders.push_back( new Oscillator() );
}

#pragma mark - MultiWave

Waves::MultiWave::MultiWave()
: FragShader("multiwave", "multiwave.frag")
{
    mNumBands = 8;
    mGlowWidth = 0.33333;
    mGlowLength = 0.7692;
    mIntensity = 1.6;
    mWaveRate = 0.6;
    mCurvature = 0.14286;
    mSeparation = 0.2;
}

void Waves::MultiWave::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    
    interface->addParam(CreateIntParam("bands", &mNumBands)
                        .minValue(1)
                        .maxValue(64));
    
    interface->addParam(CreateFloatParam("GlowWidth", &mGlowWidth));
    
    interface->addParam(CreateFloatParam("GlowLength", &mGlowLength)
                        .maxValue(3.0));
    interface->addParam(CreateFloatParam("Intensity", &mIntensity)
                        .maxValue(3.0));
    interface->addParam(CreateFloatParam("WaveRate", &mWaveRate)
                        .maxValue(2.0));
    interface->addParam(CreateFloatParam("Curvature", &mCurvature)
                        .maxValue(1.0));
    interface->addParam(CreateFloatParam("Separation", &mSeparation));
}

void Waves::MultiWave::update(double dt)
{
    
}

void Waves::MultiWave::setCustomParams( AudioInputHandler& audioInputHandler )
{
    mShader.uniform("iNumBands", mNumBands);
    mShader.uniform("iGlowWidth", mGlowWidth);
    mShader.uniform("iGlowLength", mGlowLength);
    mShader.uniform("iIntensity", mIntensity);
    mShader.uniform("iWaveRate", mWaveRate);
    mShader.uniform("iCurvature", mCurvature);
    mShader.uniform("iSeparation", mSeparation);
}


#pragma mark - SineWave

Waves::SineWave::SineWave()
: FragShader("sine", "sines.frag")
{
    
}

void Waves::SineWave::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
}

void Waves::SineWave::update(double dt)
{
    
}

void Waves::SineWave::setCustomParams( AudioInputHandler& audioInputHandler )
{
}

#pragma mark - Oscilloscope

Waves::Oscilloscope::Oscilloscope()
: FragShader("scope", "audiosignal.frag")
{
    
}

void Waves::Oscilloscope::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
}

void Waves::Oscilloscope::update(double dt)
{
    
}

void Waves::Oscilloscope::setCustomParams( AudioInputHandler& audioInputHandler )
{
}


#pragma mark - AudioGraph

Waves::AudioGraph::AudioGraph()
: FragShader("graph", "audiograph.frag")
{
    mSmoothness = 1.6f;
    mLength = 7.0f;
}

void Waves::AudioGraph::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    
    interface->addParam(CreateFloatParam("smoothness", &mSmoothness)
                        .minValue(0.1f)
                        .maxValue(9.0f));
    interface->addParam(CreateFloatParam("length", &mLength)
                        .minValue(1.0f)
                        .maxValue(10.0f));
}

void Waves::AudioGraph::setCustomParams( AudioInputHandler& audioInputHandler )
{
    mShader.uniform("iSmoothness", mSmoothness);
    mShader.uniform("iLength", mLength);
}

#pragma mark - Oscillator

Waves::Oscillator::Oscillator()
: FragShader("oscillator", "oscillator_frag.glsl")
{
    
}

void Waves::Oscillator::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    interface->addParam(CreateFloatParam( "amplitude", &mAmplitude )
                         .minValue(0.0f)
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam( "phase", &mFrequency )
                         .minValue(0.0f)
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    interface->addParam(CreateBoolParam( "audio-amp", &mAudioAmp )
                         .oscReceiver(getName()));
    interface->addParam(CreateBoolParam( "audio-phase", &mAudioPhase )
                         .oscReceiver(getName()));
}

void Waves::Oscillator::update(double dt)
{
    for (int i = 0; i < MAX_WAVES; ++i)
    {
        //        mWaveParams[i].mAudioInputHandler.update(dt, mApp->getAudioInput());
        mFrequency += mTimeScale * dt;
    }
}

void Waves::Oscillator::setCustomParams( AudioInputHandler& audioInputHandler )
{
    mShader.uniform( "iOffset", mWaveParams[0].mOffset);
    mShader.uniform( "iPhase", mFrequency);
    mShader.uniform( "iAmplitude", mAmplitude);
}

