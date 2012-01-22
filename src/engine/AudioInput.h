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
#include "KissFFT.h"

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
    
    KissRef getFft() { return mFft; }
    
    float* getTimeData() const { return mTimeData; }
    float* getFrequencyData() const { return mFrequencyData; }
    
    void resetTempoAnalyzer();
    
private:
    audio::Input* mInput;
    audio::PcmBuffer32fRef mPcmBuffer;
    
    std::shared_ptr<float> mFftDataRef;
    unsigned int mFftBandCount;
    
    // Analyzer
	bool mFftInit;
	KissRef mFft;
    
	// Data
	int32_t mDataSize;
	int32_t mInputSize;
	float*  mInputData;
	float*  mFrequencyData;
    float*  mTimeData;
    
    // Tempo
    int32_t mFirstPeak;
	int32_t mNeighbors;
	std::vector<int32_t> mPeakDistances;
	int32_t mSampleDistance;
	float mTempo;
	std::vector<float> mTempos;
	std::map<float, int32_t> mTempoScore;
	float mThreshold;
    
};

#endif