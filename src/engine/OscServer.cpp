//
//  OscServer.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 12-01-22.
//  Copyright (c) 2012 ewerx. All rights reserved.
//

#include "cinder/Cinder.h"
#include "cinder/app/App.h"
#include "OscServer.h"
#include "OscParam.h"
#include "MidiOut.h"
#include <iostream>
#include <vector>

using namespace ci;
using namespace ci::app;

// constructor
//
OscServer::OscServer()
: mIsListening(false)
, mUseThread(false)
, mIsSending(false)
, mDebugPrint(true)
, mMidiInput(NULL)
, mMidiLearning(false)
, mMidiLearnTarget(NULL)
{
}

// destructor
//
OscServer::~OscServer()
{
}

// init
//
void OscServer::setup( Config& config, MidiInput* midiInput )
{
    mListenPort = config.getInt("osc_listen_port"); 
    try 
    {
        mListener.setup(mListenPort);
    }
    catch (...) 
    {
        console() << "[osc] ERROR listening on port " << mListenPort << std::endl;
        return;
    }
    
    mIsListening = true;
    console() << "[osc] listening on port " << mListenPort << std::endl;
    
    if( mUseThread )
    {
        mThread = std::thread(&OscServer::threadLoop, this);
    }
    
    setDestination( DEST_INTERFACE, config.getString("osc_interface_ip"), config.getInt("osc_interface_port") );
    setDestination( DEST_RESOLUME, config.getString("osc_resolume_ip"), config.getInt("osc_resolume_port") );
    
    mMidiInput = midiInput;
    if( mMidiInput && mMidiInput->isEnabled() )
    {
        mCbMidiEvent = mMidiInput->registerMidiCallback(this, &OscServer::handleMidiMessage);
    }
}

void OscServer::shutdown()
{
    if (mUseThread)
    {
        mThread.join();
    }
    mIsListening = false;
    mListener.shutdown();
    while( !mIncomingCallbackQueue.empty() )
        mIncomingCallbackQueue.pop();
    
    if( mMidiInput != NULL )
    {
        mMidiInput->unregisterMidiCallback(mCbMidiEvent);
    }
}

#pragma MARK Listener

void OscServer::update()
{
    // main loop
    if( mIsListening )
    {
        if( mUseThread )
        {
            // perform callbacks on main thread
            while( !mIncomingCallbackQueue.empty() )
            {
                tIncomingCommand& cmd = mIncomingCallbackQueue.front();
                cmd.second( cmd.first );
                mIncomingCallbackQueue.pop();
            }
        }
        else
        {
            while( mListener.hasWaitingMessages() ) 
            {
                osc::Message message;
                mListener.getNextMessage(&message);
                
                processMessage(message);
            }
        }
    }
}

void OscServer::threadLoop()
{
    ThreadSetup threadSetup;
    
    while (mIsListening) 
    {
        while (mListener.hasWaitingMessages()) 
        {
            osc::Message message;
            mListener.getNextMessage(&message);
            
            processMessage(message);
        }
    }
}

void OscServer::processMessage(osc::Message& message)
{
    if( mDebugPrint )
    {
        printMessage(message, MSG_RECV, LOGLEVEL_QUIET);
    }
    
    tOscMap::iterator it = mCallbackMap.find(message.getAddress());
    if( it != mCallbackMap.end() )
    {
        tOscCallback callback = it->second;
        
        if( mUseThread )
        {
            if( mDebugPrint )
            {
                console() << "[osc] callback queued" << std::endl;
            }
            mIncomingCallbackQueue.push( tIncomingCommand(message,callback) );
        }
        else
        {
            if( mDebugPrint )
            {
                console() << "[osc] callback triggered" << std::endl;
            }
            callback(message);
        }
    }
    else
    {
        // relay to resolume
        sendMessage(message, DEST_RESOLUME, LOGLEVEL_QUIET);
    }
}

void OscServer::printMessage(const osc::Message& message, const eMessageType msgType, const eLogLevel loglevel)
{
    if( loglevel == LOGLEVEL_SILENT )
    {
        return;
    }
    
    switch(msgType)
    {
        case MSG_SENT:
            console() << "[osc] SENT: ";
            break;
        case MSG_RECV:
            console() << "[osc] RECV: ";
            break;
            
        case MSG_RELAY:
            console() << "[osc] FWD: ";
            break;
    }
    console() << message.getAddress();
    
    if( loglevel == LOGLEVEL_VERBOSE )
    {
        console() << std::endl;
        console() << "[osc] Num Arg: " << message.getNumArgs() << std::endl;
        for (int i = 0; i < message.getNumArgs(); i++) 
        {
            console() << "[osc] -- Argument " << i << std::endl;
            console() << "[osc] ---- type: " << message.getArgTypeName(i) << std::endl;
            
            switch(message.getArgType(i))
            {
                case osc::TYPE_INT32:
                {
                    try 
                    {
                        console() << "[osc] ------ value: "<< message.getArgAsInt32(i) << std::endl;
                    }
                    catch (...) 
                    {
                        console() << "[osc] Exception reading argument as int32" << std::endl;
                    }
                    
                }
                    break;
                    
                case osc::TYPE_FLOAT:
                {
                    try 
                    {
                        console() << "[osc] ------ value: " << message.getArgAsFloat(i) << std::endl;
                    }
                    catch (...) 
                    {
                        console() << "[osc] Exception reading argument as float" << std::endl;
                    }
                }
                    break;
                    
                case osc::TYPE_STRING:
                {
                    try 
                    {
                        console() << "[osc] ------ value: " << message.getArgAsString(i).c_str() << std::endl;
                    }
                    catch (...) 
                    {
                        console() << "[osc] Exception reading argument as string" << std::endl;
                    }
                }
                    break;
                    
                default:
                    console() << "[osc] Unhandled argument type: " << message.getArgType(i) << std::endl;
                    break;
            }
        }
    }
    else
    {
        for (int i = 0; i < message.getNumArgs(); i++) 
        {
            switch(message.getArgType(i))
            {
                case osc::TYPE_INT32:
                    console() << " " << message.getArgAsInt32(i);
                            break;
                    
                case osc::TYPE_FLOAT:
                    console() << " " << message.getArgAsFloat(i);
                    break;
                    
                case osc::TYPE_STRING:
                    console() << " " << message.getArgAsString(i).c_str();                    break;
                    
                default:
                    console() << " <" << message.getArgType(i) << ">";
                    break;
            }
        }
        console() << std::endl;
    }
}

#pragma MARK Sender

void OscServer::setDestination( const eDestination index, const std::string& host, const int port )
{
    if( index < DEST_COUNT )
    {
        try
        {
            mSender[index].setup( host, port );
            mIsSending = true;
            console() << "[osc] send destination " << (int)(index) << ": " << host << ":" << port << std::endl;
        }
        catch (...)
        {
            console() << "[osc] ERROR creating destination socket " << host << ":" << port << std::endl;
            return;
        }
    }
}

void OscServer::sendMessage( ci::osc::Message& message, const eDestination dest, const eLogLevel loglevel )
{
    if( mIsSending )
    {
        if( dest < DEST_ALL )
        {
            mSender[dest].sendMessage( message );
        }
        else
        {
            for( int i=0; i < DEST_COUNT; ++i )
            {
                mSender[i].sendMessage( message );
            }
        }
        
        if( mDebugPrint )
        {
            printMessage(message, (dest == DEST_RESOLUME) ? MSG_RELAY : MSG_SENT, loglevel);
        }
    }
}

#pragma mark - MIDI

bool OscServer::handleMidiMessage(MidiEvent midiEvent)
{
    tMidiAddress address = std::make_pair( midiEvent.getChannel(), midiEvent.getNote() );
    
    tMidiMap::iterator it = mMidiCallbackMap.find(address);
    
    if (mMidiLearning)
    {
        if( it != mMidiCallbackMap.end() )
        {
            console() << "[midi] removed mapping for [" << address.first << "," << address.second << "]\n";
            // remove old mapping
            mMidiCallbackMap.erase(it);
        }
        
        mMidiCallbackMap[address] = mMidiLearnCallback;
        //mMidiLearning = false;
        
        mMidiLearnTarget->setMidiAddress(address.first, address.second);
        mMidiLearnTarget->sendValue();
        
        // change color
        //sendMidiControlChange(1, address.second, 80);
        
        console() << "[midi] learned mapping for [" << address.first << "," << address.second << "]\n";
        
        return true;
    }
    
    if( it != mMidiCallbackMap.end() )
    {
        tOscCallback callback = it->second;
        
        osc::Message message;
        message.addFloatArg( midiEvent.getValueRatio() );
        
        callback(message);
        return true;
    }
    
    return false;
}

void OscServer::sendMidiControlChange(int channel, int control, int value)
{
    // TODO: refactor
    if (!mMidiInput)
        return;
    
    midi::MidiOut* midiOut = mMidiInput->getMidiOut();
    if (!midiOut)
        return;
    
    midiOut->sendControlChange(channel, control, value);
    console() << "[midi] sent CC " << channel << ":" << control << ":" << value << std::endl;
}
