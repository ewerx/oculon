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
,mBattery(0.0f)
,mBlink(0.0f)
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
,mHasData(false)
,mIsCollectingData(false)
,mUseThread(false)
,mEnableLogging(false)
,mEnableBlinkDetection(true)
,TG_GetDriverVersion(NULL)
,TG_GetNewConnectionId(NULL)
,TG_Connect(NULL) 
,TG_ReadPackets(NULL)
,TG_GetValue(NULL)
,TG_GetValueStatus(NULL)
,TG_Disconnect(NULL)
,TG_FreeConnection(NULL)
,TG_SetDataLog(NULL)
,TG_EnableBlinkDetection(NULL)
{
}

MindWave::~MindWave()
{
}

void MindWave::setup()
{
    CFURLRef bundleUrl = CFBundleCopyResourceURL(CFBundleGetMainBundle(), CFSTR("ThinkGear"), CFSTR("bundle"), CFSTR(""));
    
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
        TG_GetValueStatus       = (TGGetValueStatusPtr)CFBundleGetFunctionPointerForName(mThinkGearBundle,CFSTR("TG_GetValueStatus"));
        TG_Disconnect           = (TGDisconnectPtr)CFBundleGetFunctionPointerForName(mThinkGearBundle,CFSTR("TG_Disconnect"));
        TG_FreeConnection       = (TGFreeConnectionPtr)CFBundleGetFunctionPointerForName(mThinkGearBundle,CFSTR("TG_FreeConnection"));
        TG_SetDataLog           = (TGSetDataLogPtr)CFBundleGetFunctionPointerForName(mThinkGearBundle,CFSTR("TG_SetDataLog"));
        TG_EnableBlinkDetection = (TGEnableBlinkDetectionPtr)CFBundleGetFunctionPointerForName(mThinkGearBundle,CFSTR("TG_EnableBlinkDetection"));
        
        assert( TG_GetDriverVersion && TG_GetNewConnectionId && TG_Connect &&
               TG_ReadPackets && TG_GetValue && TG_GetValueStatus && TG_Disconnect && TG_FreeConnection && TG_SetDataLog && TG_EnableBlinkDetection );
    }
    
    if( 0 == setupNewConnection() )
    {
        if( mUseThread )
        {
            mIsCollectingData = true;
            mThread = std::thread(&MindWave::threadLoop, this);
        }
    }
}

void MindWave::shutdown()
{
    endConnection();
    CFRelease(mThinkGearBundle);
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
    
    if( mEnableLogging )
    {
        ret = TG_SetDataLog( mConnectionId, "mw_datalog.txt" );
        if( ret < 0 )
        {
            console() << "[mindwave] error setting data log path (" << ret << ")\n";
            mConnectionId = -1;
            return ret;
        }
    }
    
    console() << "[mindwave] connecting to " << mPortName << "...\n";
    
    ret = TG_Connect(mConnectionId, mPortName, TG_BAUD_57600, TG_STREAM_PACKETS);// why was this 9600 in docs? 
    
    if( ret < 0 )
    {
        console() << "[mindwave] connection failed\n";
        endConnection();
        return ret;
    }
    
    console() << "[mindwave] connected.\n";
    
    if( mEnableBlinkDetection )
    {
        TG_EnableBlinkDetection(mConnectionId, 1);
    }
    
    return ret;
}

void MindWave::endConnection()
{
    if( mIsCollectingData )
    {
        console() << "[mindwave] disconnected" << std::endl;
    }
    
    TG_FreeConnection(mConnectionId);
    mConnectionId = -1;
    mIsCollectingData = false;
    
    mThread.join();
}

void MindWave::getData()
{
    int numPackets = TG_ReadPackets(mConnectionId, -1);
    //console() << "[mindwave] getData " << numPackets << " packets in stream.\n";
    mHasData = numPackets > 0;
    if( mHasData )
    {
        mSignalQuality = TG_GetValue(mConnectionId, TG_DATA_POOR_SIGNAL);
        mBattery = TG_GetValue(mConnectionId, TG_DATA_BATTERY);
        if( mSignalQuality > 180 )
        {
            mHasData = false;
        }
        
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
        if( mEnableBlinkDetection && TG_GetValueStatus(mConnectionId, TG_DATA_BLINK_STRENGTH) > 0)
        {
            mBlink = TG_GetValue(mConnectionId, TG_DATA_BLINK_STRENGTH);
        }
        else
        {
            mBlink = 0.0f;
        }
    }
    
}
