//
//  AudioInputHandler.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2013-05-17.
//
//

#pragma once

#include "AudioInput.h"

#include "cinder/gl/Fbo.h"
#include "cinder/Timeline.h"

#include <vector>

class Interface;
class Scene;

class AudioInputHandler
{
public:
    AudioInputHandler();
    virtual ~AudioInputHandler();
    
    void setup(bool fboEnabled =false);
    void setupInterface( Interface *interface, const std::string &name, const int midiChannel =-1, const int midiNote =-1, const int randomizeMidiChannel =-1, const int randomizeMidiNote =-1 );
    void update(double dt, AudioInput& audioInput);
    
    void drawDebug(const ci::Vec2f& size);
    
    // accessors
    bool hasTexture() const { return mAudioFboEnabled; }
    ci::gl::Texture getTexture()
    {
        if (mAudioFboEnabled) return mAudioFbo.getTexture();
        else return ci::gl::Texture();
    }
    ci::gl::Fbo& getFbo() { return mAudioFbo; }
    
    float getGain() { return mGain; }
    
    // AVG VOLUME
    float getAverageVolumeByFrequencyRange(const float minRatio, const float maxRatio);
    float getAverageVolumeByFrequencyRange(const int minBand =0, const int maxBand =DEFAULT_FREQ_BIN_SIZE);
    float getAverageVolumeLowFreq() const { return mAvgVolume[BAND_LOW].mValue; }
    float getAverageVolumeMidFreq() const { return mAvgVolume[BAND_MID].mValue; }
    float getAverageVolumeHighFreq() const { return mAvgVolume[BAND_HIGH].mValue; }
    float getAverageVolumeByBand(const int bandIndex);

    // RAW FFT VALUES
    struct tFftValue
    {
        int32_t mBandIndex;
        bool mFalling;
        ci::Anim<float> mValue;
        
        tFftValue( int32_t index, float value ) : mBandIndex(index), mValue(value) { mFalling = false; }
        tFftValue() : mBandIndex(0), mValue(0.0f), mFalling(false) {}
    };
    typedef std::vector<tFftValue> FftValues;
    FftValues::const_iterator   fftValuesBegin() const     { return mFftFalloff.begin(); }
    FftValues::const_iterator   fftValuesEnd() const       { return mFftFalloff.end(); }
    FftValues& getFftValues()                              { return mFftFalloff; }
    
private:
    // callback
    bool onRandomize();
    
public:
    // Texture
    bool                mAudioFboEnabled;
    int                 mAudioFboDim;
    ci::gl::Fbo         mAudioFbo;
    
    // Distribution
    bool                mRandomSignal;
    bool                mRandomize;
    int                 mFramesUntilRandomize;
    int                 mFramesSinceRandom;
    int                 mRandomSeed;

    // FALLOFF
#define AUDIO_FALLOFF_MODE_TUPLE \
AUDIO_FALLOFF_MODE_ENTRY( "None", FALLOFF_NONE ) \
AUDIO_FALLOFF_MODE_ENTRY( "Linear", FALLOFF_LINEAR ) \
AUDIO_FALLOFF_MODE_ENTRY( "Out-Quad", FALLOFF_OUTQUAD ) \
AUDIO_FALLOFF_MODE_ENTRY( "Out-Expo", FALLOFF_OUTEXPO ) \
AUDIO_FALLOFF_MODE_ENTRY( "Out-Back", FALLOFF_OUTBACK ) \
AUDIO_FALLOFF_MODE_ENTRY( "Out-Bounce", FALLOFF_OUTBOUNCE ) \
//AUDIO_FALLOFF_MODE_ENTRY( "OutIn-Expo", FALLOFF_OUTINEXPO ) \
//AUDIO_FALLOFF_MODE_ENTRY( "OutIn-Back", FALLOFF_OUTINBACK ) \
//end tuple
    
    enum eFalloffMode
    {
#define AUDIO_FALLOFF_MODE_ENTRY( nam, enm ) \
enm,
        AUDIO_FALLOFF_MODE_TUPLE
#undef  AUDIO_FALLOFF_MODE_ENTRY
        
        FALLOFFMODE_COUNT
    };
    eFalloffMode        mFalloffMode;

    FftValues           mFftFalloff;
    float               mFalloffTime;
    bool                mFalloffByFreq;
    
    typedef std::function<float (float)> tEaseFn;
    tEaseFn getFalloffFunction();
    tEaseFn getReverseFalloffFunction();
    
    // Filtering
    float   mGain;
    bool    mLinearScale;
    bool    mFalloffAveragesOnly;
    float   mLowPassFilter;
    float   mHighPassFilter;
    
    enum eBands
    {
        BAND_LOW,
        BAND_MID,
        BAND_HIGH,
        
        BAND_COUNT,
        BAND_NONE = BAND_COUNT
    };
    tFftValue   mAvgVolume[BAND_COUNT];
    
    static std::vector<std::string>& getBandNames();
    
    // OSC Tracks
#define AUDIO_SOURCE_TUPLE \
AUDIO_SOURCE_ENTRY( "Audio", SOURCE_AUDIO ) \
AUDIO_SOURCE_ENTRY( "OSC-Track1", SOURCE_TRACK1 ) \
AUDIO_SOURCE_ENTRY( "OSC-Track2", SOURCE_TRACK2 ) \
AUDIO_SOURCE_ENTRY( "OSC-Track3", SOURCE_TRACK3 ) \
AUDIO_SOURCE_ENTRY( "OSC-Track4", SOURCE_TRACK4 ) \
//end tuple
    
    enum eInputSource
    {
#define AUDIO_SOURCE_ENTRY( nam, enm ) \
enm,
        AUDIO_SOURCE_TUPLE
#undef  AUDIO_SOURCE_ENTRY
        
        AUDIO_SOURCE_COUNT
    };
    eInputSource mInputSource;
    bool mOSCFalloff;
    
};
