/*
 *  AudioSignal.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#pragma once

#include "cinder/Cinder.h"
#include "Scene.h"
#include "cinder/Timeline.h"
#include "MotionBlurRenderer.h"
#include "AudioInputHandler.h"
#include "AudioDrawUtils.h"

#include <vector>

class SubScene;

//
// Audio input tests
//
class AudioSignal : public Scene
{
public:
    AudioSignal();
    virtual ~AudioSignal();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    bool handleKeyDown(const ci::app::KeyEvent& keyEvent);
    AudioInputHandler& getAudioInputHandler() { return mAudioInputHandler; }
    
protected:
    void setupInterface();
    //void setupDebugInterface();
    
private:
    void drawSubScenes();
    //void drawWaveform   ( ci::audio::PcmBuffer32fRef pcmBufferRef );
    void drawSpectrum( const std::vector<float>& magSpectrum );
    
    bool setFilter();
    bool removeFilter();
    
private:
    MotionBlurRenderer  mMotionBlurRenderer;
    bool                mUseMotionBlur;
    
    //TODO: audio2 cleanup
    // kiss
    int mFilter;
    float mFilterFrequency;
    
    // sub scenes
    std::vector<SubScene*> mSubScenes;
    
    AudioInputHandler mAudioInputHandler;
    
    SpectrumPlot mSpectrumPlot;
};

