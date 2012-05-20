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
#include "Constants.h"
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/bind.hpp>

typedef boost::function<void(const ci::osc::Message&)> tOscCallback;
typedef boost::unordered_map<std::string, tOscCallback> tOscMap;

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
    
    void setup();
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
    
private:
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
    ci::osc::Listener       mListener;
    int                     mListenPort;
    bool                    mIsListening;
    tOscMap                 mCallbackMap;
    
    boost::thread           mThread;
    bool                    mUseThread;
    
    //TODO: multiple destinations
    ci::osc::Sender         mSender[DEST_COUNT];
    std::string             mSendHost;
    int                     mSendPort;
    bool                    mIsSending;
    
    bool                    mDebugPrint;
    
    static const int    LISTEN_PORT;
    static const char*  INTERFACE_IP;
    static const int    INTERFACE_PORT;
    static const char*  LOCALHOST_IP;
    static const int    RESOLUME_PORT;
};

#endif