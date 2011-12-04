/*
 *  MidiMap.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-26.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __MIDIMAP_H__
#define __MIDIMAP_H__

#include "MidiInput.h"
#include <string>
#include <map>

using namespace ci;
using namespace std;

typedef std::function<void(MidiEvent)> tMidiCmdCallback;

class MidiMap
{
public:
    MidiMap();
    virtual ~MidiMap();
    
    //TODO: fix this
    void init(MidiInput* midiInput);
    
    // registerMidiEvent( string id, eEventType type, callback )
    template<typename T>
    void registerMidiEvent( string name, int eventType, T* obj, void (T::*callback)(MidiEvent) )
    {
        if( mIsLearning )
        {
            assert( false && "why are you registering in learning mode?" );
            return;
        }
        
        mCmdMap.insert(std::make_pair(name, 
                                      MidiCommand(static_cast<MidiEvent::eEventType>(eventType), 
                                                  std::bind1st( std::mem_fun(callback), obj)) ) );
    }
    
    void unregisterMidiEvent( string name );
    
    // callback from MidiInput
    bool handleMidiMessage( MidiEvent midiEvent );
    
    void beginLearning();
    void cancelLearning()       { mIsLearning = false; }
    
private:
    enum 
    { 
        UNDEFINED_CHANNEL = -1,
        UNDEFINED_NOTE = -1,
    };
    
    struct MidiCommand
    {
        //int mPort; // assuming only 1 controller
        int                     mChannel;
        int                     mNote;
        MidiEvent::eEventType   mType;
        tMidiCmdCallback        mCallback;
        
        MidiCommand()
        {
            mChannel = UNDEFINED_CHANNEL;
            mNote = UNDEFINED_NOTE;
            mType = MidiEvent::TYPE_BUTTON_PRESS;
        }
        
        MidiCommand(MidiEvent::eEventType eventType, tMidiCmdCallback cb)
        {
            mType = eventType;
            mChannel = UNDEFINED_CHANNEL;
            mNote = UNDEFINED_NOTE;
            mCallback = cb;
        }
        
        MidiCommand(MidiEvent::eEventType eventType, int channel, int note, tMidiCmdCallback cb)
        {
            mType = eventType;
            mChannel = channel;
            mNote = note;
            mCallback = cb;
        }
    };
    
    MidiInput* mMidiInput;
    CallbackId mCbMidiEvent;
    
    //TODO: serialization
    std::map<string,MidiCommand> mCmdMap;
        
    bool mIsLearning;
    std::map<string,MidiCommand>::iterator mLearningIterator;
    
    
    typedef std::map<string,MidiMap::MidiCommand> tMidiMapCmds;
};

#endif // __MIDIMAP_H__
