/*
 *  MidiInput.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-25.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __MIDIINPUT_H__
#define __MIDIINPUT_H__

#include "cinder/Cinder.h"
#include "MidiHub.h"

using namespace ci;

class MidiInput
{
public:
    MidiInput();
    virtual ~MidiInput();
    
    void update();
    
private:
    midi::Hub   mMidiHub;
    
    // track notes in map structure
};

#endif // __MIDIINPUT_H__
