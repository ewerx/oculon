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
:mEnabled(true)
{
}

MidiInput::~MidiInput()
{
}

void MidiInput::update()
{
    if( mEnabled )
    {
        midi::Message msg;
        
        while (mMidiHub.getNextMessage(&msg))
        {
            if( DEBUG_MIDI )
            {
                console() << "[midi] port: " << msg.port << " ch: " << msg.channel << " status: " << msg.status;
                console() << " note: " << msg.byteOne << " val: " << msg.byteTwo << std::endl;
            }
            
            mCallbacksMidi.call( MidiEvent(msg) );
            /*
            if( mLearnMode )
            {
                

            }
            else 
            {
                
            
            switch (msg.status) 
            {
                case MIDI_CONTROL_CHANGE:
                    //TODO: look up in map structure, update current value
                    break;
            }
            }
            */
        }
    }
}
