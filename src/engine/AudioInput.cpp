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
#include <boost/bind.hpp>

using namespace ci;
using namespace ci::app;
using namespace ci::audio;

// constructor
//
AudioInput::AudioInput()
: mInput(NULL)
, mFftBandCount(FftProcessor::DEFAULT_BAND_COUNT)
, mFftLogPlot(KISS_DEFAULT_DATASIZE)
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
	//initialize the audio Input, using the default input device
    //TODO: specify audio input, change at run-time
	mInput = new audio::Input( /*devices.front()*/ );
    
    mFftInit = false;
    mGain = 1.0f;
}

void AudioInput::setupInterface( Interface* interface )
{
    assert( interface );
    
    //iterate input devices
	const std::vector<audio::InputDeviceRef>& devices = audio::Input::getDevices();
    
    char oscCmdBuf[128];
    
    int index = 0;
    console() << "[audio] input devices found:\n";
	for( std::vector<audio::InputDeviceRef>::const_iterator iter = devices.begin();
        iter != devices.end(); ++iter )
    {
        snprintf(oscCmdBuf, 128, "audioinput%d", index);
		console() << '\t' << (*iter)->getName() << std::endl;
        interface->addButton(CreateTriggerParam((*iter)->getName(), NULL)
                             .oscReceiver("master", oscCmdBuf))->registerCallback( boost::bind( &AudioInput::changeInput, this, index) );
        ++index;
	}
    
    interface->addParam(CreateFloatParam("Input Gain", &mGain)
                        .maxValue(50.0f)
                        .oscReceiver("master", "gain")
                        .sendFeedback());
}

bool AudioInput::changeInput( const int index )
{
    const std::vector<audio::InputDeviceRef>& devices = audio::Input::getDevices();
    
    int i = 0;
    console() << "[audio] input devices found:\n";
	for( std::vector<audio::InputDeviceRef>::const_iterator iter = devices.begin();
        iter != devices.end(); ++iter )
    {
        if( i == index )
        {
            if( mInput )
            {
                mInput->stop();
                delete mInput;
                mInput = NULL;
            }
            mInput = new audio::Input( *iter );
            //mFftInit = false;
            break;
        }
        i++;
    }
    
    return false;
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
    if( !mInput)
    {
        return;
    }
    
    if( !mInput->isCapturing() )
    {
        //tell the input to start capturing audio
        mInput->start();
        console() << "[audio] capturing input from device: " << mInput->getDefaultDevice()->getName() << std::endl;
    }
    else
    {
        mPcmBuffer = mInput->getPcmBuffer();
        
        if( !mPcmBuffer ) 
        {
            console() << "[audio] no pcm buffer\n";
            return;
        }
        
        //KISS
        {
            if ( mPcmBuffer->getInterleavedData() )
            {
                
                // Get sample count
                uint32_t sampleCount = mPcmBuffer->getInterleavedData()->mSampleCount;
                if ( sampleCount > 0 ) {
                    
                    // Kiss is not initialized
                    if ( !mFftInit ) 
                    {
                        console() << "[audio] KISS initialized (" << sampleCount << " bands)" << std::endl;
                        // Initialize analyzer
                        mFftInit = true;
                        mFft = Kiss::create( /*sampleCount*/ );
                        
                        for( int i=0; i < sampleCount; ++i )
                        {
                            mFftLogPlot.push_back(Vec2f::zero());
                        }
                    }
                    
                    // Analyze data
                    if (mPcmBuffer->getInterleavedData()->mData != 0) 
                    {
                        mInputData = mPcmBuffer->getInterleavedData()->mData;
                        mInputSize = mPcmBuffer->getInterleavedData()->mSampleCount;
                        mFft->setData( mInputData );
                    }
                    
                    analyzeKissFft();

                    
                }
            }
        }
        
        if( mPcmBuffer->getSampleCount() > 0 )
        {
            //presently FFT only works on OS X, not iOS or Windows
            //mFftDataRef = audio::calculateFft( mPcmBuffer->getInterleavedData(), mFftBandCount );
            mFftDataRef = audio::calculateFft( mPcmBuffer->getChannelData( CHANNEL_FRONT_LEFT ), mFftBandCount );
        }
    }
}

void AudioInput::analyzeKissFft()
{
    // Get data
    float * freqData = mFft->getAmplitude();
    int32_t dataSize = mFft->getBinSize();
    
    // Iterate through data
    for (int32_t i = 0; i < dataSize; i++) 
    {
        // Do logarithmic plotting for frequency domain
        double mLogSize = log((double)dataSize);
        mFftLogPlot[i].x = (float)(log((double)i) / mLogSize) * (double)dataSize;
        mFftLogPlot[i].y = math<float>::clamp(freqData[i] * (mFftLogPlot[i].x / dataSize) * log((double)(dataSize - i)), 0.0f, 2.0f) * mGain;
    }
}

float AudioInput::getAverageVolumeByFrequencyRange(const int minBand /*=0*/, const int maxBand /*=512*/)
{
    float amplitude = 0.0f;
    
    int minIndex = math<int>::max( 0, minBand );
    int maxIndex = math<int>::min( mFft->getBinSize(), maxBand );
    
    for (int32_t i = minIndex; i < maxIndex; i++)
    {
        amplitude += mFftLogPlot[i].y;
    }
    
    amplitude = amplitude / (float)maxIndex;
    
    return amplitude;
}
