//
//  AudioBandSelector.cpp
//  Oculon1
//
//  Created by Ehsan Rezaie on 2015-05-20.
//
//

#include "AudioBandSelector.h"
#include "Interface.h"

AudioBandSelector::AudioBandSelector()
: EnumSelector()
{
    mNames.push_back("low");
    mNames.push_back("mid");
    mNames.push_back("high");
    mNames.push_back("none");
    
    mIndex = mNames.size()-1;
}

void AudioBandSelector::setupInterface(Interface *interface, const std::string &name)
{
    interface->addEnum(CreateEnumParam(name, &mIndex)
                       .maxValue(mNames.size())
                       .isVertical()
                       .oscReceiver(name)
                       .sendFeedback(), mNames);
}
