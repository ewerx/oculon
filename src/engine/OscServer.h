//
//  OscServer.h
//  Oculon
//
//  Created by Ehsan Rezaie on 12-01-22.
//  Copyright (c) 2012 ewerx. All rights reserved.
//

#ifndef __OSCSERVER_H__
#define __OSCSERVER_H__

#include "OscListener.h"
#include "OscSender.h"
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/bind.hpp>

typedef boost::function<void(const ci::osc::Message&)> tOscCallback;
typedef boost::unordered_map<std::string, tOscCallback> tOscMap;

class OscServer
{
public:
    OscServer();
    ~OscServer();
    
    void setup();
    void shutdown();
    void update();
    
    // listener
    template<typename T>
    void registerCallback( std::string address, T* obj, void (T::*callback)(const ci::osc::Message&) )
    {
        mCallbackMap[address] = boost::bind( callback, obj, boost::arg<1>() );
    }
    
    void unregisterCallback( const std::string& address )
    {
        mCallbackMap.erase(address);
    }
    
    // sender
    void setDestination( const std::string& host, const int port );
    void sendMessage( ci::osc::Message& message );
    
private:
    void threadLoop();
    void processMessage(const ci::osc::Message& message);
    void printMessage( const ci::osc::Message& message, const bool sent );
    
private:
    ci::osc::Listener       mListener;
    int                     mListenPort;
    bool                    mIsListening;
    tOscMap                 mCallbackMap;
    
    boost::thread           mThread;
    bool                    mUseThread;
    
    //TODO: multiple destinations
    ci::osc::Sender         mSender;
    std::string             mSendHost;
    int                     mSendPort;
    bool                    mIsSending;
    
    bool                    mDebugPrint;
};

#endif