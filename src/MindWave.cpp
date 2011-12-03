/*
 *  MindWave.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-30.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "cinder/app/App.h"
#include "MindWave.h"
#include "cinder/cocoa/CinderCocoa.h"

using namespace ci;
using namespace ci::app;


MindWave::MindWave()
:mConnectionId(-1)
,mPortName("/dev/tty.MindWave")
,mSignalQuality(0.0f)
,mAttention(0.0f) // eSense attention
,mMeditation(0.0f) // eSense meditation
,mRaw(0.0f)
,mDelta(0.0f)
,mTheta(0.0f)
,mAlpha1(0.0f)
,mAlpha2(0.0f)
,mBeta1(0.0f)
,mBeta2(0.0f)
,mGamma1(0.0f)
,mGamma2(0.0f)
,mIsCollectingData(false)
,mUseThread(false)
,TG_GetDriverVersion(NULL)
,TG_GetNewConnectionId(NULL)
,TG_Connect(NULL) 
,TG_ReadPackets(NULL)
,TG_GetValue(NULL)
,TG_Disconnect(NULL)
,TG_FreeConnection(NULL)
,TG_SetDataLog(NULL)
{
    for(int i=0; i < BAND_COUNT; ++i)
    {
        mBand[i] = 0.0f;
    }
    
    CFURLRef bundleUrl = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, CFSTR("/Library/ThinkGear.bundle"),
                                                       kCFURLPOSIXPathStyle, true);
    
    mThinkGearBundle = CFBundleCreate(kCFAllocatorDefault, bundleUrl);
    
    CFRelease(bundleUrl); 
    
    assert(mThinkGearBundle != NULL && "could not load ThinkGear.bundle");
    
    if( mThinkGearBundle )
    {
        TG_GetDriverVersion     = (TGFunctionPtr)CFBundleGetFunctionPointerForName(mThinkGearBundle, CFSTR("TG_GetDriverVersion"));
        TG_GetNewConnectionId   = (TGFunctionPtr)CFBundleGetFunctionPointerForName(mThinkGearBundle, CFSTR("TG_GetNewConnectionId"));
        TG_Connect              = (TGConnectPtr)CFBundleGetFunctionPointerForName(mThinkGearBundle, CFSTR("TG_Connect"));
        TG_ReadPackets          = (TGReadPacketsPtr)CFBundleGetFunctionPointerForName(mThinkGearBundle,CFSTR("TG_ReadPackets"));
        TG_GetValue             = (TGGetValuePtr)CFBundleGetFunctionPointerForName(mThinkGearBundle,CFSTR("TG_GetValue"));
        TG_Disconnect           = (TGDisconnectPtr)CFBundleGetFunctionPointerForName(mThinkGearBundle,CFSTR("TG_Disconnect"));
        TG_FreeConnection       = (TGFreeConnectionPtr)CFBundleGetFunctionPointerForName(mThinkGearBundle,CFSTR("TG_FreeConnection"));
        TG_SetDataLog           = (TGSetDataLogPtr)CFBundleGetFunctionPointerForName(mThinkGearBundle,CFSTR("TG_SetDataLog"));
        
        assert( TG_GetDriverVersion && TG_GetNewConnectionId && TG_Connect && 
               TG_ReadPackets && TG_GetValue && TG_Disconnect && TG_FreeConnection && TG_SetDataLog );
    }
}

MindWave::~MindWave()
{
    endConnection();
    CFRelease(mThinkGearBundle);
}

void MindWave::setup()
{
    if( 0 == setupNewConnection() )
    {
        if( mUseThread )
        {
            mIsCollectingData = true;
            mThread = boost::thread(&MindWave::threadLoop, this);
        }
    }
}

void MindWave::threadLoop()
{
    console() << "[mindwave] thread launched...\n";
    cocoa::SafeNsAutoreleasePool autorelease;
    while( mIsCollectingData )
    {
        // sleep for half a second
		usleep(50000);
        
        getData();
    }
    console() << "[mindwave] thread terminated\n";
}

void MindWave::update()
{
    if( mConnectionId != -1 )
    {
        if( !mUseThread )
            getData();
    }
}

int MindWave::setupNewConnection()
{
    int ret = -1;
    mConnectionId = TG_GetNewConnectionId();
    
    ret = TG_SetDataLog( mConnectionId, "mw_datalog.txt" );
    if( ret < 0 )
    {
        console() << "[mindwave] error setting data log path (" << ret << ")\n";
        mConnectionId = -1;
        return ret;
    }
    
    console() << "[mindwave] connecting to " << mPortName << "... ";
    
    ret = TG_Connect(mConnectionId, mPortName, TG_BAUD_57600, TG_STREAM_PACKETS);// why was this 9600 in docs? 
    
    console() << ((ret == 0) ? "connected.\n" : "failed.\n");
    
    if( ret < 0 )
    {
        endConnection();
    }
    
    return ret;
}

void MindWave::endConnection()
{
    TG_FreeConnection(mConnectionId);
    mConnectionId = -1;
    mIsCollectingData = false;
    
    mThread.join();
    
    console() << "[mindwave] disconnected.\n";
}

void MindWave::getData()
{
    int numPackets = TG_ReadPackets(mConnectionId, -1);
    //console() << "[mindwave] getData " << numPackets << " packets in stream.\n";
    
    if( numPackets > 0 )
    {
        mSignalQuality = TG_GetValue(mConnectionId, TG_DATA_POOR_SIGNAL);
        mAttention = TG_GetValue(mConnectionId, TG_DATA_ATTENTION);
        mMeditation = TG_GetValue(mConnectionId, TG_DATA_MEDITATION);
        
        mRaw = TG_GetValue(mConnectionId, TG_DATA_RAW);
        mDelta = TG_GetValue(mConnectionId, TG_DATA_DELTA);
        mTheta = TG_GetValue(mConnectionId, TG_DATA_THETA);
        mAlpha1 = TG_GetValue(mConnectionId, TG_DATA_ALPHA1);
        mAlpha2 = TG_GetValue(mConnectionId, TG_DATA_ALPHA2);
        mBeta1 = TG_GetValue(mConnectionId, TG_DATA_BETA1);
        mBeta2 = TG_GetValue(mConnectionId, TG_DATA_BETA2);
        mGamma1 = TG_GetValue(mConnectionId, TG_DATA_GAMMA1);
        mGamma2 = TG_GetValue(mConnectionId, TG_DATA_GAMMA2);
    }
    
}
