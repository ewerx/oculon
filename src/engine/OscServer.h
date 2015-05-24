//
//  OscServer.h
//  Oculon
//
//  Created by Ehsan Rezaie on 12-01-22.
//  Copyright (c) 2012 ewerx. All rights reserved.
//

#ifndef __OSCSERVER_H__
#define __OSCSERVER_H__

#include "MidiInput.h"
#include "OscListener.h"
#include "OscSender.h"
#include "Constants.h"
#include "Config.h"
#include "cinder/Thread.h"
#include <boost/unordered_map.hpp>
#include <boost/bind.hpp>
#include <queue>

class OscParam;

typedef std::function<void(const ci::osc::Message&)> tOscCallback;
typedef boost::unordered_map<std::string, tOscCallback> tOscMap;
typedef std::pair<int,int> tMidiAddress;
typedef boost::unordered_map<tMidiAddress, tOscCallback> tMidiMap;

class OscServer
{
public:
    enum eDestination
    {
        DEST_INTERFACE,
        DEST_RESOLUME,
        
        DEST_ALL,
        DEST_COUNT = DEST_ALL
    };
public:
    OscServer();
    ~OscServer();
    
    void setup( Config& config, MidiInput *midiInput =NULL );
    void shutdown();
    void update();
    
    // listener
    template<typename T>
    void registerCallback( std::string address, T* obj, void (T::*callback)(const ci::osc::Message&) )
    {
        mCallbackMap[address] = boost::bind( callback, obj, boost::arg<1>() );
    }
    
    void registerCallback( std::string address, tOscCallback callback )
    {
        mCallbackMap[address] = callback;
    }
    
    void unregisterCallback( const std::string& address )
    {
        mCallbackMap.erase(address);
    }
    
    // sender
    void setDestination( const eDestination index, const std::string& host, const int port );
    void sendMessage( ci::osc::Message& message, const eDestination dest =DEST_INTERFACE, const eLogLevel loglevel =LOGLEVEL_QUIET );
    
    //TODO: refactor
    // midi listener
    template<typename T>
    void registerMidiCallback( tMidiAddress address, T* obj, void (T::*callback)(const ci::osc::Message&) )
    {
        mMidiCallbackMap[address] = boost::bind( callback, obj, boost::arg<1>() );
    }
    
    void registerMidiCallback( tMidiAddress address, tOscCallback callback )
    {
        mMidiCallbackMap[address] = callback;
    }
    
    void unregisterMidiCallback( tMidiAddress address )
    {
        mMidiCallbackMap.erase(address);
    }
    
    // midi sender
    void sendMidiControlChange( int channel, int control, int value );

    // callback from MidiInput
    bool handleMidiMessage( MidiEvent midiEvent );
    
    void learnMidi( OscParam* target, tOscCallback callback )
    {
        mMidiLearning = true;
        mMidiLearnTarget = target;
        mMidiLearnCallback = callback;
    }
    void stopLearning() { mMidiLearning = false; };
    
    bool                    mMidiLearning;
    
private:
    typedef std::pair<ci::osc::Message,tOscCallback> tIncomingCommand;
    
    enum eMessageType
    {
        MSG_SENT,
        MSG_RECV,
        MSG_RELAY
    };
    
    void threadLoop();
    void processMessage(ci::osc::Message& message);
    void printMessage( const ci::osc::Message& message, const eMessageType type, const eLogLevel loglevel );
    
private:
    ci::osc::Listener               mListener;
    int                             mListenPort;
    bool                            mIsListening;
    tOscMap                         mCallbackMap;
    std::queue<tIncomingCommand>    mIncomingCallbackQueue;
    
    std::thread             mThread;
    bool                    mUseThread;
    
    //TODO: multiple destinations
    ci::osc::Sender         mSender[DEST_COUNT];
    std::string             mSendHost;
    int                     mSendPort;
    bool                    mIsSending;
    
    bool                    mDebugPrint;
    
    // MIDI
    MidiInput*              mMidiInput;
    ci::CallbackId          mCbMidiEvent;
    tMidiMap                mMidiCallbackMap;
    tOscCallback            mMidiLearnCallback;
    OscParam*               mMidiLearnTarget;
};

#endif