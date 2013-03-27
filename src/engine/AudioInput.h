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
#include "cinder/Vector.h"
#include "KissFFT.h"
#include "Interface.h"

#define KISS_DEFAULT_DATASIZE 256

class AudioInput
{
public:
    typedef std::vector<ci::Vec2f> FftLogPlot;
public:
    AudioInput();
    ~AudioInput();
    
    void setup();
    void shutdown();
    void update();
    
    // accessors
    ci::audio::PcmBuffer32fRef getPcmBuffer()   { return mPcmBuffer; }
    std::shared_ptr<float> getFftDataRef()  { return mFftDataRef; }
    
    unsigned int getFftBandCount() const { return mFftBandCount; }
    void setFftBandCount(const unsigned int count) { mFftBandCount = count; }
    
    KissRef getFft() { return mFft; }
    FftLogPlot::const_iterator   fftPlotBegin() const     { return mFftLogPlot.begin(); }
    FftLogPlot::const_iterator   fftPlotEnd() const       { return mFftLogPlot.end(); }
    const FftLogPlot& getFftLogData() const { return mFftLogPlot; }
    
    float getAverageVolumeByFrequencyRange(const int minBand =0, const int maxBand =KISS_DEFAULT_DATASIZE);
    
    void setupInterface( Interface* interface );
    bool changeInput( const int index );
    
private:
    void analyzeKissFft();
    
    ci::audio::Input* mInput;
    ci::audio::PcmBuffer32fRef mPcmBuffer;
    
    std::shared_ptr<float> mFftDataRef;
    unsigned int mFftBandCount;
    
    float mGain;
    
    // Kiss Analyzer
	bool    mFftInit;
	KissRef mFft;
    FftLogPlot mFftLogPlot;
    
	// Data
	int32_t mDataSize;
	int32_t mInputSize;
	float * mInputData;
	float * mTimeData;
};

#endif