//
//  AudioBandSelector.h
//  Oculon1
//
//  Created by Ehsan Rezaie on 2015-05-20.
//
//

#pragma once

#include "EnumSelector.h"
#include "NamedObject.h"
#include "AudioInputHandler.h"

class Interface;

class AudioBandSelector : public EnumSelector
{
public:
    AudioBandSelector(bool slider =false);
    
    void setupInterface(Interface *interface, const std::string& name);
    
    bool mSlider;
};

class AudioFloatParam : public oculon::NamedObject<AudioFloatParam>
{
public:
    AudioFloatParam(const std::string &name, float defValue, float minValue, float maxValue, bool median =false)
    : NamedObject(name)
    , mValue(defValue)
    , mMinValue(minValue)
    , mMaxValue(maxValue)
    , mMedian(median)
    , mBand(true) {}
    
    //float operator()() const              { return mValue; }
    float operator()(AudioInputHandler & audioInputHandler) const {
        if (mMedian)
        {
            return mValue * ( 0.5f + 0.5f * audioInputHandler.getAverageVolumeByBand(mBand()));
        }
        else
        {
            return mValue * audioInputHandler.getAverageVolumeByBand(mBand());
        }
    }
    
    void setupInterface(Interface *interface, const std::string& name);
    
    float mValue;
    float mMinValue;
    float mMaxValue;
    bool mMedian;
    AudioBandSelector mBand;
};