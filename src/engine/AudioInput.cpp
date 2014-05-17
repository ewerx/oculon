/*
 *  AudioInput.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-16.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "cinder/Cinder.h"
#include "cinder/app/App.h"
#include "cinder/audio/Context.h"
#include "cinder/audio/Device.h"
#include "cinder/audio/ScopeNode.h"
#include "cinder/audio/Utilities.h"
#include "AudioInput.h"
#include <iostream>
#include <vector>
#include <boost/bind.hpp>

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace ci::audio;

// constructor
//
AudioInput::AudioInput()
: mGain(1.0f)
{
}

// destructor
//
AudioInput::~AudioInput()
{
}

// init
//
void AudioInput::setup()
{
	//initialize the audio Input, using the default input device
	auto ctx = audio::Context::master();
    
	mInputDeviceNode = ctx->createInputDeviceNode();
    
    audio::Device::printDevices();
    
	reset();
}

void AudioInput::reset()
{
    auto ctx = audio::Context::master();
    auto scopeFmt = audio::ScopeSpectralNode::Format().fftSize( 1024 ).windowSize( 1024 );
	mScopeSpectralNode = ctx->makeNode( new audio::ScopeSpectralNode( scopeFmt ) );
    mScopeNode = ctx->makeNode( new audio::ScopeNode() );
    
    mInputDeviceNode >> mScopeSpectralNode;
    mInputDeviceNode >> mScopeNode;
    
    mInputDeviceNode->enable();
	ctx->enable();
    
    mFftInit = false;
    mGain = 1.0f;
    
    for (int i = 0; i < NUM_TRACKS; ++i)
    {
        mLiveTrackData[i].mHighLevel = 0.0f;
        mLiveTrackData[i].mMidLevel = 0.0f;
        mLiveTrackData[i].mLowLevel = 0.0f;
    }
    
    console() << "[audio] input source: " << mInputDeviceNode->getDevice()->getName() << endl;
}

void AudioInput::setupInterface( Interface* interface )
{
    assert( interface );
    
    //iterate input devices
    
    string oscName;
    
    int index = 0;
    console() << "[audio] available input sources :\n";
	for( const auto& device : Device::getInputDevices() )
    {
        oscName = "audioinput" + to_string(index);
		console() << '\t' << device->getName() << std::endl;
        interface->gui()->addLabel(device->getName());
//        interface->addButton(CreateTriggerParam(device->getName(), NULL)
//                             .oscReceiver("master", oscName))->registerCallback( boost::bind( &AudioInput::changeInput, this, index) );
        ++index;
	}
    
    interface->addParam(CreateFloatParam("Global Input Gain", &mGain)
                        .maxValue(10.0f)
                        .oscReceiver("master", "gain")
                        .sendFeedback()
                        .midiInput(0,2,23));
}

bool AudioInput::changeInput( const int index )
{
    auto ctx = audio::Context::master();
    auto devices = Device::getInputDevices();
    
    if ( mInputDeviceNode->getDevice() == devices[index] )
    {
        return true;
    }
    
    console() << "[audio] switching to input " << index << ": " << devices[index]->getName() << endl;
    
    mInputDeviceNode->disable();
    mScopeNode->disconnectAll();
    mScopeSpectralNode->disconnectAll();
    mInputDeviceNode->disconnectAll();
//
    mInputDeviceNode = ctx->createInputDeviceNode( devices[index] );
//    mInputDeviceNode >> mScopeSpectralNode;
//    
//    mInputDeviceNode->enable();
    reset();
    
    return false;
}

void AudioInput::shutdown()
{
//    mInputDeviceNode->disconnectAll();
//    mScopeSpectralNode->disconnectAll();
//    mScopeNode->disconnectAll();
    
//    mInputDeviceNode->disable();
}

// update
//
void AudioInput::update()
{
    
}

//void AudioInput::analyzeKissFft()
//{
//    // Get data
//    float * freqData = mFft->getAmplitude();
//    int32_t dataSize = mFft->getBinSize();
//    
//    // Iterate through data
//    for (int32_t i = 0; i < dataSize; i++) 
//    {
//        // Do logarithmic plotting for frequency domain
//        double mLogSize = log((double)dataSize);
//        mFftLogPlot[i].x = (float)(log((double)i) / mLogSize) * (double)dataSize;
//        mFftLogPlot[i].y = math<float>::clamp(freqData[i] * (mFftLogPlot[i].x / dataSize) * log((double)(dataSize - i)), 0.0f, 2.0f) * mGain;
//    }
//}

float AudioInput::getAverageVolumeByFrequencyRange(const float minRatio, const float maxRatio)
{
    return getAverageVolumeByFrequencyRange( (int)(minRatio * KISS_DEFAULT_DATASIZE), (int)(maxRatio * KISS_DEFAULT_DATASIZE) );
}

float AudioInput::getAverageVolumeByFrequencyRange(const int minBand /*=0*/, const int maxBand /*=256*/)
{
    float amplitude = 0.0f;
    
    int minIndex = math<int>::max( 0, minBand );
    int maxIndex = math<int>::min( mFft->getBinSize(), maxBand );
    
    for (int32_t i = minIndex; i < maxIndex; i++)
    {
        amplitude += mFftLogPlot[i].y;
    }
    
    amplitude = amplitude / (float)(maxIndex-minIndex);
    
    return amplitude;
}

#pragma mark - OSC LiveGrabber Handlers

void AudioInput::setupLiveGrabberInput(OscServer &oscServer)
{
    oscServer.registerCallback( "/HiFollower", this, &AudioInput::handleHighOsc );
    oscServer.registerCallback( "/MidFollower", this, &AudioInput::handleMidOsc );
    oscServer.registerCallback( "/LoFollower", this, &AudioInput::handleLowOsc );
}

// TODO: send same message and use param for specifying low/mid/high
void AudioInput::handleHighOsc(const ci::osc::Message &message)
{
    if( message.getNumArgs() == 2 )
    {
        int track = message.getArgAsInt32(0);
        float val = message.getArgAsFloat(1);
        mLiveTrackData[track-1].mHighLevel = val;
    }
}

void AudioInput::handleMidOsc(const ci::osc::Message &message)
{
    if( message.getNumArgs() == 2 )
    {
        int track = message.getArgAsInt32(0);
        float val = message.getArgAsFloat(1);
        mLiveTrackData[track-1].mMidLevel = val;
    }
}

void AudioInput::handleLowOsc(const ci::osc::Message &message)
{
    if( message.getNumArgs() == 2 )
    {
        int track = message.getArgAsInt32(0);
        float val = message.getArgAsFloat(1);
        mLiveTrackData[track-1].mLowLevel = val;
    }
}
