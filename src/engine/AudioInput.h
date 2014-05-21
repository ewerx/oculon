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
#include "cinder/audio/ScopeNode.h"
#include "cinder/audio/GainNode.h"
#include "cinder/Vector.h"
#include "Interface.h"
#include "OscServer.h"

#define DEFAULT_FREQ_BIN_SIZE 512

class AudioInput
{
public:
    typedef std::vector<ci::Vec2f> FftLogPlot;
public:
    AudioInput();
    ~AudioInput();
    
    void setup();
    void reset();
    void shutdown();
    void update();
    
    // accessors
    
    // TODO: rename
    unsigned int getFftBandCount() const { return mScopeSpectralNode->getNumBins(); }
    
    const std::vector<float>& getMagSpectrum()              { return mScopeSpectralNode->getMagSpectrum(); }
    const ci::audio::Buffer& getBuffer() { return mScopeNode->getBuffer(); }
    
    float getAverageVolumeByFrequencyRange(const float minRatio =0.0f, const float maxRatio =1.0f);
    float getAverageVolumeByFrequencyRange(const int minBand =0, const int maxBand =DEFAULT_FREQ_BIN_SIZE);
    
    // TODO: refactor
    inline float getHighLevelForLiveTrack( const int trackIndex )
    {
        assert(trackIndex < NUM_TRACKS);
        return mLiveTrackData[trackIndex].mHighLevel;
    }
    inline float getMidLevelForLiveTrack( const int trackIndex )
    {
        assert(trackIndex < NUM_TRACKS);
        return mLiveTrackData[trackIndex].mMidLevel;
    }
    inline float getLowLevelForLiveTrack( const int trackIndex )
    {
        assert(trackIndex < NUM_TRACKS);
        return mLiveTrackData[trackIndex].mLowLevel;
    }
    
    void setupInterface( Interface* interface );
    bool changeInput( const int index );
    
    // OSC audio levels from Ableton LiveGrabber
    void setupLiveGrabberInput( OscServer& oscServer );
    void handleHighOsc( const ci::osc::Message& message );
    void handleMidOsc( const ci::osc::Message& message );
    void handleLowOsc( const ci::osc::Message& message );
    
    // callbacks
    bool onGainChanged();
    
private:
    ci::audio::InputDeviceNodeRef	mInputDeviceNode;
    ci::audio::ScopeNodeRef         mScopeNode;
    ci::audio::ScopeSpectralNodeRef	mScopeSpectralNode;
    
    float mGain;
    ci::audio::GainNodeRef mGainNode;
    
    // LiveGrabber Data
    enum { NUM_TRACKS = 4 };
    struct tLiveTrackData
    {
        float mHighLevel;
        float mMidLevel;
        float mLowLevel;
    };
    tLiveTrackData mLiveTrackData[NUM_TRACKS];
};

#endif