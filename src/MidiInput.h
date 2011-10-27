/*
 *  MidiInput.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-25.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __MIDIINPUT_H__
#define __MIDIINPUT_H__

#include "cinder/Cinder.h"
#include "cinder/Function.h"
#include "cinder/app/Event.h"
#include "MidiConstants.h"
#include "MidiHub.h"

using namespace ci;
using namespace ci::app;

#define DEBUG_MIDI 1

//
// MidiEvent for Midi callbacks
//
class MidiEvent : public Event
{
public:
    enum eEventType 
    {
        TYPE_UNKNOWN = -1,
        TYPE_BUTTON_PRESS = MIDI_NOTE_ON,
        TYPE_VALUE_CHANGE = MIDI_CONTROL_CHANGE,
    };
public:
    MidiEvent(const midi::Message& midiMsg)
        : Event(), mMessage(midiMsg) 
    {}
    
    int getChannel() const      { return mMessage.channel; }
    int getNote() const         { return mMessage.byteOne; }
    int getValue() const        { return mMessage.byteTwo; }
    int getType() const         { return mMessage.status; }
    float getValueRatio() const { return( mMessage.byteTwo / 127.f ); }
    
private:
    const midi::Message& mMessage;
};

//
// Midi input manager
//
class MidiInput
{
public:
    MidiInput();
    virtual ~MidiInput();
    
    void update();
    
    template<typename T>
    CallbackId registerMidiCallback( T* obj, bool (T::*callback)(MidiEvent) )
    {
        return mCallbacksMidi.registerCb( std::bind1st( std::mem_fun(callback), obj) );
    }

    void unregisterMidiCallback( CallbackId id ) { mCallbacksMidi.unregisterCb( id ); }
    

private:
    
    midi::Hub   mMidiHub;
                    
    CallbackMgr<bool(MidiEvent)> mCallbacksMidi;
    
    //queue<tMidiKey*>    mLearningQueue;
    //vector<tMidiKey*>   mKnownKeys;
    
};

#endif // __MIDIINPUT_H__
