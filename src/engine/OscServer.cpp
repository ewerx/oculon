//
//  OscServer.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 12-01-22.
//  Copyright (c) 2012 ewerx. All rights reserved.
//

#include "cinder/Cinder.h"
#include "cinder/app/App.h"
#include "cinder/audio/FftProcessor.h"
#include "OscServer.h"
#include <iostream>
#include <vector>

using namespace ci;
using namespace ci::app;
using namespace ci::audio;

// constructor
//
OscServer::OscServer()
: mIsListening(false)
, mUseThread(true)
, mIsSending(false)
, mDebugPrint(true)
{
}

// destructor
//
OscServer::~OscServer()
{
}

// init
//
void OscServer::setup( Config& config )
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
        mThread = boost::thread(&OscServer::threadLoop, this);
    }
    
    setDestination( DEST_INTERFACE, config.getString("osc_interface_ip"), config.getInt("osc_interface_port") );
    setDestination( DEST_RESOLUME, config.getString("osc_resolume_ip"), config.getInt("osc_resolume_port") );
}

void OscServer::shutdown()
{
    mIsListening = false;
    mListener.shutdown();
    while( !mIncomingCallbackQueue.empty() )
        mIncomingCallbackQueue.pop();
}

#pragma MARK Listener

void OscServer::update()
{
    // main loop
    if( mIsListening )
    {
        if( mUseThread )
        {
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
        mSender[index].setup( host, port );
        mIsSending = true;
        console() << "[osc] send destination " << (int)(index) << ": " << host << ":" << port << std::endl;
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
