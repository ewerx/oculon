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
    
private:
    audio::Input* mInput;
    std::shared_ptr<float> mFftDataRef;
    audio::PcmBuffer32fRef mPcmBuffer;
    unsigned int mFftBandCount;
};

#endif