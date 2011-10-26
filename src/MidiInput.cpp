/*
 *  MidiInput.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-25.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "MidiInput.h"
#include "cinder/app/App.h"

using namespace ci;
using namespace ci::app;

MidiInput::MidiInput()
{
}

MidiInput::~MidiInput()
{
}

void MidiInput::update()
{
    midi::Message msg;
    
    while (mMidiHub.getNextMessage(&msg))
    {
        int note = msg.byteOne;
        int val = msg.byteTwo;
        
        switch (msg.status) 
        {
            case MIDI_CONTROL_CHANGE:
                //TODO: look up in map structure, update current value
                break;
        }
        
        console() << "[midi] port: " << msg.port << " ch: " << msg.channel << " status: " << msg.status;
        console() << " note: " << note << " val: " << val << std::endl;
    }
}
