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

#define DEFAULT_LISTEN_PORT 8889
#define DEFAULT_SEND_PORT 8080

using namespace ci;
using namespace ci::app;
using namespace ci::audio;

// constructor
//
OscServer::OscServer()
: mListenPort(DEFAULT_LISTEN_PORT)
, mIsListening(false)
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
void OscServer::setup()
{
    mListener.setup(mListenPort);
    mIsListening = true;
    console() << "[osc] listening on port " << mListenPort << std::endl;
    
    if( mUseThread )
    {
        mThread = boost::thread(&OscServer::threadLoop, this);
    }
    
    setDestination( "192.168.1.148", 8888 );
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

void OscServer::processMessage(const osc::Message& message)
{
    if( mDebugPrint )
    {
        printMessage(message, false);
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
}

void OscServer::printMessage(const osc::Message& message, const bool sent)
{
    if( sent )
    {
        console() << "[osc] Message sent" << std::endl;
    }
    else
    {
        console() << "[osc] Message received" << std::endl;
    }
    console() << "[osc] Address: " << message.getAddress() << std::endl;
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

#pragma MARK Sender

void OscServer::setDestination( const std::string& host, const int port )
{
    mSender.setup( host, port );
    mIsSending = true;
    console() << "[osc] send destination: " << host << ":" << port << std::endl;
}

void OscServer::sendMessage( ci::osc::Message& message )
{
    if( mIsSending )
    {
        mSender.sendMessage( message );
        if( mDebugPrint )
        {
            printMessage(message, true);
        }
    }
}
