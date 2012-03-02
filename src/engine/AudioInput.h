/*
 *  AudioInput.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-16.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __AUDIOINPUT_H__
#define __AUDIOINPUT_H__

#include "cinder/Cinder.h"
#include "cinder/audio/Input.h"
#include "Kiss.h"

using namespace ci;

class AudioInput
{
public:
    AudioInput();
    ~AudioInput();
    
    void setup();
    void shutdown();
    void update();
    
    // accessors
    audio::PcmBuffer32fRef getPcmBuffer()   { return mPcmBuffer; }
    std::shared_ptr<float> getFftDataRef()  { return mFftDataRef; }
    
    unsigned int getFftBandCount() const { return mFftBandCount; }
    void setFftBandCount(const unsigned int count) { mFftBandCount = count; }
    
    Kiss& getFft() { return mFft; }
    
private:
    audio::Input* mInput;
    audio::PcmBuffer32fRef mPcmBuffer;
    
    std::shared_ptr<float> mFftDataRef;
    unsigned int mFftBandCount;
    
    // Analyzer
	bool mFftInit;
	Kiss mFft;
    
	// Data
	int32_t mDataSize;
	int32_t mInputSize;
	float * mInputData;
	float * mTimeData;
};

#endif