//
//  OscServer.h
//  Oculon
//
//  Created by Ehsan Rezaie on 12-01-22.
//  Copyright (c) 2012 ewerx. All rights reserved.
//

#ifndef __OSCSERVER_H__
#define __OSCSERVER_H__

#include "cinder/Cinder.h"
#include "OscListener.h"
#include <boost/thread.hpp>

using namespace ci;

class OscServer
{
public:
    OscServer();
    ~OscServer();
    
    void setup();
    void shutdown();
    void update();
    
    osc::Listener& getListener()    { return mListener; }
    
private:
    void threadLoop();
    void printMessage( const osc::Message& message );
    
private:
    osc::Listener mListener;
    int mListenPort;
    
    // thread
    boost::thread           mThread;
    bool                    mIsListening;
    bool                    mUseThread;
};

#endif