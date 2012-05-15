/*
 *  AudioTest.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __AUDIOTEST_H__
#define __AUDIOTEST_H__

#include "cinder/Cinder.h"
#include "cinder/audio/Input.h"
#include "Scene.h"
#include "cinder/Timeline.h"
#include "MotionBlurRenderer.h"

//
// Audio input tests
//
class AudioTest : public Scene
{
public:
    AudioTest() : Scene("audio") {}
    virtual ~AudioTest() {}
    
    // inherited from Scene
    void setup();
    //void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    
protected:
    void setupInterface();
    
private:
    void drawWaveform   ( ci::audio::PcmBuffer32fRef pcmBufferRef );
    void drawFft        ( std::shared_ptr<float> fftDataRef );
    void drawLines      ();
    
    MotionBlurRenderer  mMotionBlurRenderer;
    bool                mUseMotionBlur;
};

#endif // __AUDIOTEST_H__
