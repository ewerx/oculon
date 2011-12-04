/*
 *  MindWave.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-30.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __MINDWAVE_H__
#define __MINDWAVE_H__

#include <CoreFoundation/CoreFoundation.h>
#include <boost/thread.hpp>

#define TG_BAUD_1200 1200
#define TG_BAUD_2400 2400
#define TG_BAUD_4800 4800
#define TG_BAUD_9600 9600
#define TG_BAUD_57600 57600
#define TG_BAUD_115200 115200

#define TG_STREAM_PACKETS 0
#define TG_STREAM_5VRAW 1
#define TG_STREAM_FILE_PACKETS 2

#define TG_DATA_BATTERY 0
#define TG_DATA_POOR_SIGNAL 1
#define TG_DATA_ATTENTION 2
#define TG_DATA_MEDITATION 3
#define TG_DATA_RAW 4
#define TG_DATA_DELTA 5
#define TG_DATA_THETA 6
#define TG_DATA_ALPHA1 7
#define TG_DATA_ALPHA2 8
#define TG_DATA_BETA1 9
#define TG_DATA_BETA2 10
#define TG_DATA_GAMMA1 11
#define TG_DATA_GAMMA2 12

typedef int(*TGFunctionPtr)();
typedef int(*TGConnectPtr)(int, const char*, int, int);
typedef int(*TGReadPacketsPtr)(int,int);
typedef float(*TGGetValuePtr)(int,int);
typedef int(*TGDisconnectPtr)(int);
typedef void(*TGFreeConnectionPtr)(int);
typedef int(*TGSetDataLogPtr)(int, const char*);

class MindWave
{
public:
    enum eBand
    {
        BAND_DELTA,
        BAND_THETA,
        BAND_ALPHA_LOW,
        BAND_ALPHA_HIGH,
        BAND_BETA_LOW,
        BAND_BETA_HIGH,
        BAND_GAMMA_LOW,
        BAND_GAMMA_HIGH,
        
        BAND_COUNT
    };
    
public:
    MindWave();
    ~MindWave();
    
    void setup();
    void update();
    
    float getSignalQuality() const  { return mSignalQuality; }
    float getAttention() const      { return mAttention; }
	float getMeditation() const     { return mMeditation; }
	float getRaw() const            { return mRaw; }
	float getDelta() const          { return mDelta; }
	float getTheta() const          { return mTheta; }
	float getAlpha1() const         { return mAlpha1; }
	float getAlpha2() const         { return mAlpha2; }
	float getBeta1() const          { return mBeta1; }
	float getBeta2() const          { return mBeta2; }
	float getGamma1() const         { return mGamma1; }
	float getGamma2() const         { return mGamma2; }
    
    void threadLoop();
    
private:
    int setupNewConnection();
    void endConnection();
    void getData();
    
private:
    
    // data
    float mSignalQuality; // poor signal status
	float mAttention; // eSense attention
	float mMeditation; // eSense meditation
	float mRaw;
	float mDelta;
	float mTheta;
	float mAlpha1;
	float mAlpha2;
	float mBeta1;
	float mBeta2;
	float mGamma1;
	float mGamma2;
    float mBand[BAND_COUNT];
    
    // thread
    boost::thread           mThread;
    bool                    mIsCollectingData;
    bool                    mUseThread;
    
    // connection
    CFURLRef                mBundleUrl;
    CFBundleRef             mThinkGearBundle;
    int                     mConnectionId;
    const char*             mPortName;
    
    TGFunctionPtr       TG_GetDriverVersion;
    TGFunctionPtr       TG_GetNewConnectionId; 
    TGConnectPtr        TG_Connect;
    TGReadPacketsPtr    TG_ReadPackets;
    TGGetValuePtr       TG_GetValue;
    TGDisconnectPtr     TG_Disconnect;
    TGFreeConnectionPtr TG_FreeConnection;
    TGSetDataLogPtr     TG_SetDataLog;
};

#endif // __MINDWAVE_H__
