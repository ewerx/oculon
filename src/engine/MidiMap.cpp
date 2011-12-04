/*
 *  MidiMap.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-26.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "MidiMap.h"
#include "cinder/app/App.h"
#include <map>



MidiMap::MidiMap()
: mMidiInput(NULL)
, mIsLearning(false)
{
}

void MidiMap::init(MidiInput* midiInput)
{
    mMidiInput = midiInput;
    mCbMidiEvent = midiInput->registerMidiCallback(this, &MidiMap::handleMidiMessage);
}

MidiMap::~MidiMap()
{
    mCmdMap.clear();
    if( mMidiInput != NULL )
        mMidiInput->unregisterMidiCallback(mCbMidiEvent);
}

// register function defined in header

void MidiMap::unregisterMidiEvent(string name)
{
    tMidiMapCmds::iterator it = mCmdMap.find(name);
    if( it != mCmdMap.end() )
    {
        mCmdMap.erase( it );
    }
    else 
    {
        assert(false && "midi event not found");
    }
}

bool MidiMap::handleMidiMessage(MidiEvent midiEvent)
{
    if( mIsLearning )
    {
        if (mLearningIterator != mCmdMap.end()) 
        {
            MidiCommand& cmdToLearn = (*mLearningIterator).second;
            if( cmdToLearn.mType == midiEvent.getType() )
            {
                console() << "[midi] event " << (*mLearningIterator).first << " learned.\n";                
                cmdToLearn.mChannel = midiEvent.getChannel();
                cmdToLearn.mNote = midiEvent.getNote();
                ++mLearningIterator;
                if( mLearningIterator != mCmdMap.end() )
                    console() << "[midi] now listening for event: " << (*mLearningIterator).first << "...\n";
                else 
                {
                    mIsLearning = false;
                    console() << "[midi] learning mode completed\n";
                }

            }
            else 
            {
                if( DEBUG_MIDI )
                {
                    console() << "[midi] warning: command " << (*mLearningIterator).first << " not learned. mismatched midi event type.\n";
                }
            }

        }
        else 
        {
            mIsLearning = false;
            console() << "[midi] learning mode completed\n";
        }
    }
    else // doing
    {
        for (tMidiMapCmds::iterator it = mCmdMap.begin(); it != mCmdMap.end(); ++it)
        {
            const MidiCommand& cmd = (*it).second;
            
            if (cmd.mChannel == midiEvent.getChannel() &&
                cmd.mNote == midiEvent.getNote() &&
                cmd.mType == midiEvent.getType() )
            {
                if( DEBUG_MIDI )
                {
                    console() << "\tmapped midi event " << (*it).first << " found!\n";
                }
                cmd.mCallback( midiEvent );
                break;
            }
        }
    }
    
    return false;
}

void MidiMap::beginLearning()
{
    mIsLearning = true;
    
    //TODO: interactive learning, for now do them all in sequence
    mLearningIterator = mCmdMap.begin();
    
    console() << "[midi] entered learning mode...\n";
    console() << "[midi] now listening for command: " << (*mLearningIterator).first << "...\n";
}
