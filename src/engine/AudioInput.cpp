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
    
    console() << "[audio] devices:\n" << audio::Device::printDevicesToString();
    
	reset();
}

void AudioInput::reset()
{
    auto ctx = audio::Context::master();
    auto scopeFmt = audio::MonitorSpectralNode::Format().fftSize( 1024 ).windowSize( 1024 );
	mMonitorSpectralNode = ctx->makeNode( new audio::MonitorSpectralNode( scopeFmt ) );
    mMonitorNode = ctx->makeNode( new audio::MonitorNode() );
    
    mGainNode = ctx->makeNode( new audio::GainNode() );
    mGain = 1.0f;
    mGainNode->setValue(mGain);
    
    mInputDeviceNode >> mGainNode >> mMonitorSpectralNode;
    mInputDeviceNode >> mGainNode >> mMonitorNode;
    
    mInputDeviceNode->enable();
	ctx->enable();
    
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
                        .midiInput(0,1,1))->registerCallback(this, &AudioInput::onGainChanged);
}

bool AudioInput::onGainChanged()
{
    mGainNode->setValue(mGain);
    return true;
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
    mMonitorNode->disconnectAll();
    mMonitorSpectralNode->disconnectAll();
    mInputDeviceNode->disconnectAll();
//
    mInputDeviceNode = ctx->createInputDeviceNode( devices[index] );
//    mInputDeviceNode >> mMonitorSpectralNode;
//    
//    mInputDeviceNode->enable();
    reset();
    
    return false;
}

void AudioInput::shutdown()
{
//    mInputDeviceNode->disconnectAll();
//    mMonitorSpectralNode->disconnectAll();
//    mMonitorNode->disconnectAll();
    
//    mInputDeviceNode->disable();
}

// update
//
void AudioInput::update()
{
    
}

float AudioInput::getAverageVolumeByFrequencyRange(const float minRatio, const float maxRatio)
{
    return getAverageVolumeByFrequencyRange( (int)(minRatio * mMonitorSpectralNode->getNumBins()), (int)(maxRatio * mMonitorSpectralNode->getNumBins()) );
}

float AudioInput::getAverageVolumeByFrequencyRange(const int minBand /*=0*/, const int maxBand /*=256*/)
{
    float amplitude = 0.0f;
    
    int minIndex = math<int>::max( 0, minBand );
    int maxIndex = math<int>::min( mMonitorSpectralNode->getNumBins(), maxBand );
    
    for (int32_t i = minIndex; i < maxIndex; i++)
    {
        amplitude += mMonitorSpectralNode->getMagSpectrum()[i];
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
