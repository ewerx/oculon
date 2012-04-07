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
    
    ci::osc::Listener& getListener()    { return mListener; }
    
    template<typename T>
    void registerCallback( std::string address, T* obj, void (T::*callback)(const ci::osc::Message&) )
    {
        mCallbackMap[address] = boost::bind( callback, obj, boost::arg<1>() );//std::bind1st( std::mem_fun(callback), obj );
    }
    
    void unregisterCallback( const std::string& address )
    {
        mCallbackMap.erase(address);
    }
    
private:
    void threadLoop();
    void processMessage(const ci::osc::Message& message);
    void printMessage( const ci::osc::Message& message );
    
private:
    ci::osc::Listener       mListener;
    int                     mListenPort;
    
    // thread
    boost::thread           mThread;
    bool                    mIsListening;
    bool                    mUseThread;
    
    tOscMap                 mCallbackMap;
    
    bool                    mDebugPrint;
};

#endif