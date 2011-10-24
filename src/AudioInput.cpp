/*
 *  AudioInput.cpp
 *  OculonProto
 *
 *  Created by Ehsan on 11-10-16.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "cinder/Cinder.h"
#include "cinder/app/AppBasic.h"
#include "cinder/audio/FftProcessor.h"
#include "AudioInput.h"
#include <iostream>
#include <vector>

using namespace ci;
using namespace ci::app;

// constructor
//
AudioInput::AudioInput()
{
}

// destructor
//
AudioInput::~AudioInput()
{
}

// init
//
void AudioInput::Init(ci::app::AppBasic* mainApp)
{
    //iterate input devices and print their names to the console
	const std::vector<audio::InputDeviceRef>& devices = audio::Input::getDevices();
	for( std::vector<audio::InputDeviceRef>::const_iterator iter = devices.begin(); 
        iter != devices.end(); ++iter ) 
    {
		mainApp->console() << (*iter)->getName() << std::endl;
	}
    
	//initialize the audio Input, using the default input device
	mInput = audio::Input();
	
	//tell the input to start capturing audio
	mInput.start();
}

// update
//
void AudioInput::update()
{
    mPcmBuffer = mInput.getPcmBuffer();
	if( ! mPcmBuffer ) 
    {
		return;
	}
    
	uint16_t bandCount = 512;
	//presently FFT only works on OS X, not iOS or Windows
	mFftDataRef = audio::calculateFft( mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT ), bandCount );
}
