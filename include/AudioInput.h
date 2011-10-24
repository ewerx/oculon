/*
 *  AudioInput.h
 *  OculonProto
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

// fwd decls
class ci::app::AppBasic;

class AudioInput
{
public:
    AudioInput();
    ~AudioInput();
    
    void Init(ci::app::AppBasic* mainApp);
    void update();
    
    // accessors
    audio::PcmBuffer32fRef GetPcmBuffer() { return mPcmBuffer; }
    std::shared_ptr<float> GetFftDataRef() { return mFftDataRef; }
    
private:
    audio::Input mInput;
    std::shared_ptr<float> mFftDataRef;
    audio::PcmBuffer32fRef mPcmBuffer;
};

#endif