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
    mShaders.push_back( new Oscillator() );
}

#pragma mark - MultiWave

Waves::MultiWave::MultiWave()
: FragShader("multiwave", "multiwave.frag")
{
    
}

void Waves::MultiWave::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
}

void Waves::MultiWave::update(double dt)
{
    
}

void Waves::MultiWave::setCustomParams( AudioInputHandler& audioInputHandler )
{
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

