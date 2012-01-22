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
	mInput = new audio::Input( /*devices.front()*/ );
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
        
        const bool useKissFft = true;
        const bool useCinderFft = true;
        
        if( useKissFft )
        {
            if (mPcmBuffer->getInterleavedData())
            {
                // Get sample count
                uint32_t sampleCount = mPcmBuffer->getInterleavedData()->mSampleCount;
                if (sampleCount > 0)
                {
                    
                    // Initialize analyzer, if needed
                    if (!mFftInit)
                    {
                        mFftInit = true;
                        mFft = Kiss::create(sampleCount);
                        
                        // Set filter on FFT to calculate tempo based on beats
                        mFft->setFilter(0.1f, Kiss::Filter::LOW_PASS);
                    }
                    
                    // Analyze data
                    if (mPcmBuffer->getInterleavedData()->mData != 0) 
                    {
                        // set FFT data
                        mInputData = mPcmBuffer->getInterleavedData()->mData;
                        mInputSize = mPcmBuffer->getInterleavedData()->mSampleCount;
                        mFft->setData(mInputData);
                    
                        // Get data
                        mTimeData = mFft->getData();
                        mFrequencyData = mFft->getAmplitude();
                        mDataSize = mFft->getBinSize();
                        
                        // Iterate through amplitude data
                        for (int32_t i = 0; i < mDataSize; i++, mSampleDistance++) 
                        {
                            
                            // Check value against threshold
                            if (mTimeData[i] >= mThreshold)
                            {
                                
                                // Determine neighbor range
                                int32_t start = math<int32_t>::max(i - mNeighbors, 0);
                                int32_t end = math<int32_t>::min(i + mNeighbors, mDataSize - 1);
                                
                                // Compare this value with neighbors to find peak
                                bool peak = true;
                                for (int32_t j = start; j < end; j++)
                                    if (j != i && mTimeData[i] <= mTimeData[j])
                                    {
                                        peak = false;
                                        break;
                                    }
                                
                                // This is a peak
                                if (peak)
                                {
                                    
                                    // Add distance between this peak and last into the 
                                    // list. Just note position if this is the first peak.
                                    if (mFirstPeak >= 0)
                                        mPeakDistances.push_back(mSampleDistance);
                                    else
                                        mFirstPeak = mSampleDistance;
                                    
                                    // Reset distance counter
                                    mSampleDistance = 0;
                                    
                                }
                                
                            }
                            
                        }
                        
                    }

                }
                
                // We have multiple peaks to compare
                if (mPeakDistances.size() > 1)
                {
                    
                    // Add distances
                    int32_t total = 0;
                    uint32_t peakCount = mPeakDistances.size();
                    for (uint32_t i = 0; i < peakCount; i++)
                        total += mPeakDistances[i];
                    
                    // Determine tempo based on average peak distance
                    mTempo = total > 0 ? (44100.0f / ((float)total / (float)mPeakDistances.size())) * 60.0f / 1000.0f : 0.0f;
                    
                }
            }
            
            /*
             
             // Increase this tempo's score
             mTempoScore[mTempo]++;
             
             // Build a unique tempo list
             mTempos.clear();
             for (map<float, int32_t>::const_iterator tempoIt = mTempoScore.cbegin(); tempoIt != mTempoScore.cend(); ++tempoIt)
             mTempos.push_back(tempoIt->first);
             
             // Sort unique tempo vector by scores in the map. This might not work on 
             // OSX because the sort method is a lambda function. Either update XCode 
             // to use the latest compiler or refactor this to use a traditional sort
             // (pain in the ass because it needs to see the map)
             sort(mTempos.begin(), mTempos.end(), [=](const float & a, const float & b)
             {
             return mTempoScore[a] > mTempoScore[b];
             });
             
             // Set tempo to value with highest score
             mTempo = 0.0f;
             if (mTempos.size() > 0)
             mTempo = * mTempos.rbegin();
             */
        }
        
        if( useCinderFft && mPcmBuffer->getSampleCount() > 0 )
        {
            //presently FFT only works on OS X, not iOS or Windows
            //mFftDataRef = audio::calculateFft( mPcmBuffer->getInterleavedData(), mFftBandCount );
            mFftDataRef = audio::calculateFft( mPcmBuffer->getChannelData( CHANNEL_FRONT_LEFT ), mFftBandCount );
        }
    }
}
