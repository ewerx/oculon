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
#include "cinder/audio/FftProcessor.h"
#include "AudioInput.h"
#include <iostream>
#include <vector>

using namespace ci;
using namespace ci::app;
using namespace ci::audio;

// constructor
//
AudioInput::AudioInput()
: mInput(NULL)
, mFftBandCount(FftProcessor::DEFAULT_BAND_COUNT)
{
}

// destructor
//
AudioInput::~AudioInput()
{
    delete mInput;
}

// init
//
void AudioInput::setup()
{
    //iterate input devices and print their names to the console
	const std::vector<audio::InputDeviceRef>& devices = audio::Input::getDevices();
    
    console() << "[audio] input devices found:\n";
	for( std::vector<audio::InputDeviceRef>::const_iterator iter = devices.begin(); 
        iter != devices.end(); ++iter ) 
    {
		console() << '\t' << (*iter)->getName() << std::endl;
	}
    
	//initialize the audio Input, using the default input device
    //TODO: specify audio input, change at run-time
	mInput = new audio::Input( devices.front() );
}

void AudioInput::shutdown()
{
    if( mInput )
    {
        mInput->stop();
    }
}

// update
//
void AudioInput::update()
{
    if( !mInput )
    {
        return;
    }
    
    if( !mInput->isCapturing())
    {
        if( getElapsedSeconds() > 2.0f )
        {
            //tell the input to start capturing audio
            mInput->start();
            console() << "[audio] now listening...\n";
        }
    }
    else
    {
        if( getElapsedSeconds() > 3.0f )
        {
        mPcmBuffer = mInput->getPcmBuffer();
        if( !mPcmBuffer ) 
        {
            return;
        }
            
        cinder::audio::Buffer32fRef pcmBuffer =  mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT );        
        if( mPcmBuffer->getSampleCount() > 0 )
        {
            console() << "!!! got audio samples\n";
            //presently FFT only works on OS X, not iOS or Windows
            mFftDataRef = audio::calculateFft( mPcmBuffer->getInterleavedData()/*getChannelData( audio::CHANNEL_FRONT_LEFT )*/, mFftBandCount );
        }
        }
    }
}
