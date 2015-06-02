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
#include "TimeController.h"
#include "cinder/Timeline.h"

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
    AudioFloatParam()
    : NamedObject("audio-param")
    , mValue(0.0f)
    , mMinValue(0.0f)
    , mMaxValue(1.0f)
    , mMedian(false)
    , mBand(true) {}
    
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

class TimelineFloatParam : public oculon::NamedObject<TimelineFloatParam>
{
public:
    TimelineFloatParam()
    : NamedObject("timeline-param")
    , mValue(0.0f)
    , mTargetValue(0.0f)
    , mMinValue(0.0f)
    , mMaxValue(1.0f)
    , mTimeController(NULL) {}
    
    TimelineFloatParam(const std::string &name, float defValue, float minValue, float maxValue, TimeController *timeController)
    : NamedObject(name)
    , mValue(defValue)
    , mTargetValue(defValue)
    , mMinValue(minValue)
    , mMaxValue(maxValue)
    , mTimeController(timeController) {}
    
    float operator()() const              { return mValue(); }
    
    void setupInterface(Interface *interface, const std::string& name);
    
    bool onTargetValueChanged();
    
    TimeController *mTimeController;
    ci::Anim<float> mValue;
    float mTargetValue;
    float mMinValue;
    float mMaxValue;
    
};