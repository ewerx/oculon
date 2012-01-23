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

#define DEFAULT_PORT 8889

using namespace ci;
using namespace ci::app;
using namespace ci::audio;

// constructor
//
OscServer::OscServer()
: mListenPort(DEFAULT_PORT)
, mIsListening(false)
, mUseThread(false)
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
}

void OscServer::shutdown()
{
    mIsListening = false;
    mListener.shutdown();
}

void OscServer::update()
{
    if( mIsListening && !mUseThread )
    {
        while (mListener.hasWaitingMessages()) 
        {
            osc::Message message;
            mListener.getNextMessage(&message);
            
            printMessage(message);
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
            
            printMessage(message);
        }
    }
}

void OscServer::printMessage(const osc::Message& message)
{
    console() << "New message received" << std::endl;
    console() << "Address: " << message.getAddress() << std::endl;
    console() << "Num Arg: " << message.getNumArgs() << std::endl;
    for (int i = 0; i < message.getNumArgs(); i++) 
    {
        console() << "-- Argument " << i << std::endl;
        console() << "---- type: " << message.getArgTypeName(i) << std::endl;
        
        switch(message.getArgType(i))
        {
            case osc::TYPE_INT32:
            {
                try 
                {
                    console() << "------ value: "<< message.getArgAsInt32(i) << std::endl;
                }
                catch (...) 
                {
                    console() << "Exception reading argument as int32" << std::endl;
                }
                
            }
                break;
                
            case osc::TYPE_FLOAT:
            {
                try 
                {
                    console() << "------ value: " << message.getArgAsFloat(i) << std::endl;
                }
                catch (...) 
                {
                    console() << "Exception reading argument as float" << std::endl;
                }
            }
                break;
                
            case osc::TYPE_STRING:
            {
                try 
                {
                    console() << "------ value: " << message.getArgAsString(i).c_str() << std::endl;
                }
                catch (...) 
                {
                    console() << "Exception reading argument as string" << std::endl;
                }
            }
                break;
                
            default:
                console() << "Unhandled argument type: " << message.getArgType(i) << std::endl;
                break;
        }
    }
}
