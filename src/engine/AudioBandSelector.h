//
//  AudioBandSelector.h
//  Oculon1
//
//  Created by Ehsan Rezaie on 2015-05-20.
//
//

#pragma once

#include "EnumSelector.h"

class Interface;

class AudioBandSelector : public EnumSelector
{
public:
    AudioBandSelector();
    
    void setupInterface(Interface *interface, const std::string& name);
};
