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
    interface->addParam(CreateFloatParam(getName(), &mValue)
                        .minValue(mMinValue)
                        .maxValue(mMaxValue)
                        .oscReceiver(oscName));
    mBand.setupInterface(interface, getName() + "-band");
    interface->addParam(CreateBoolParam(getName() + "-median", &mMedian));
}