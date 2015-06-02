//
//  AudioBandSelector.cpp
//  Oculon1
//
//  Created by Ehsan Rezaie on 2015-05-20.
//
//

#include "AudioBandSelector.h"
#include "Interface.h"

using namespace std;
using namespace ci;
using namespace ci::app;

AudioBandSelector::AudioBandSelector(bool slider)
: EnumSelector()
, mSlider(slider)
{
    mNames.push_back("low");
    mNames.push_back("mid");
    mNames.push_back("high");
    mNames.push_back("none");
    
    mIndex = mNames.size()-1;
}

void AudioBandSelector::setupInterface(Interface *interface, const std::string &name)
{
    if (mSlider)
    {
        interface->addEnum(CreateIntParam(name, &mIndex)
                           .maxValue(mNames.size())
                           .oscReceiver(name)
                           .sendFeedback());
    }
    else
    {
        interface->addEnum(CreateEnumParam(name, &mIndex)
                           .maxValue(mNames.size())
                           .isVertical()
                           .oscReceiver(name)
                           .sendFeedback(), mNames);
    }
}

#pragma mark - AudioFloatParam

void AudioFloatParam::setupInterface(Interface *interface, const std::string &name)
{
    string oscName = name + "/" + getName();
    interface->addParam(CreateFloatParam(name, &mValue)
                        .minValue(mMinValue)
                        .maxValue(mMaxValue)
                        .oscReceiver(name));
    mBand.setupInterface(interface, name + "-band");
    interface->addParam(CreateBoolParam(name + "-median", &mMedian));
}

#pragma mark - TimelineFloatParam

void TimelineFloatParam::setupInterface(Interface *interface, const std::string &name)
{
//    string oscName = name + "/" + getName();
    
    interface->addParam(CreateFloatParam(name, &mTargetValue)
                        .minValue(mMinValue)
                        .maxValue(mMaxValue)
                        .oscReceiver(name))->registerCallback(this, &TimelineFloatParam::onTargetValueChanged);
}

bool TimelineFloatParam::onTargetValueChanged()
{
    if (mTargetValue != mValue() && mTimeController)
    {
        timeline().apply( &mValue, mTargetValue, mTimeController->getTransitionTime(), EaseOutQuad() );
    }
    
    return true;
}
