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

// statics
const int   OscServer::LISTEN_PORT      = 8889;
const char* OscServer::INTERFACE_IP     = "192.168.1.103";
const int   OscServer::INTERFACE_PORT   = 8080;
const char* OscServer::LOCALHOST_IP     = "192.168.1.140";
const int   OscServer::RESOLUME_PORT    = 9080;

// constructor
//
OscServer::OscServer()
: mIsListening(false)
, mUseThread(true)
, mIsSending(false)
, mDebugPrint(true)
{
    mListenPort = OscServer::LISTEN_PORT;
}

// destructor
//
OscServer::~OscServer()
{
}

// init
//
void OscServer::setup()
{
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
    
    setDestination( DEST_INTERFACE, INTERFACE_IP, INTERFACE_PORT );
    setDestination( DEST_RESOLUME, LOCALHOST_IP, RESOLUME_PORT );
}

void OscServer::shutdown()
{
    mIsListening = false;
    mListener.shutdown();
}

#pragma MARK Listener

void OscServer::update()
{
    if( mIsListening && !mUseThread )
    {
        while (mListener.hasWaitingMessages()) 
        {
            osc::Message message;
            mListener.getNextMessage(&message);
            
            processMessage(message);
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
        printMessage(message, MSG_RECV, LOGLEVEL_VERBOSE);
    }
    
    tOscMap::iterator it = mCallbackMap.find(message.getAddress());
    if( it != mCallbackMap.end() )
    {
        tOscCallback callback = it->second;
        callback(message);
        
        if( mDebugPrint )
        {
            console() << "[osc] Callback triggered" << std::endl;
        }
    }
    else
    {
        // relay
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
            console() << "[osc] === SENT ===" << std::endl;
            break;
        case MSG_RECV:
            console() << "[osc] === RECEIVED ===" << std::endl;
            break;
            
        case MSG_RELAY:
            console() << "[osc] === RELAYED ===" << std::endl;
            break;
    }
    console() << "[osc] Address: " << message.getAddress() << std::endl;
    
    if( loglevel == LOGLEVEL_VERBOSE )
    {
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
            printMessage(message, MSG_SENT, loglevel);
        }
    }
}
