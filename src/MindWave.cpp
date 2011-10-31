/*
 *  MindWave.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-30.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "MindWave.h"

MindWave::MindWave()
:TG_GetDV(NULL)
,TG_GetNCId(NULL)
,TG_Connect(NULL) 
{
    
    CFURLRef bundleUrl = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, CFSTR("/Library/ThinkGear.bundle"),
                                                       kCFURLPOSIXPathStyle, true);
    
    mThinkGearBundle = CFBundleCreate(kCFAllocatorDefault, bundleUrl);
    
    CFRelease(bundleUrl); 
    
    assert(mThinkGearBundle != NULL && "could not load ThinkGear.bundle");
    
    if( mThinkGearBundle )
    {
        //TODO...
        TG_GetDV    = ((void *)CFBundleGetFunctionPointerForName(mThinkGearBundle, CFSTR("TG_GetDriverVersion")));
        TG_GetNCId  = (void *)CFBundleGetFunctionPointerForName(mThinkGearBundle, CFSTR("TG_GetNewConnectionId"));
        TG_Connect  = (void *)CFBundleGetFunctionPointerForName(mThinkGearBundle, CFSTR("TG_Connect"));
        //TG_Connect = (void *)CFBundleGetFunctionPointerForName(thinkGearBundle,CFSTR("TG_Connect"));
        //TG_ReadPackets = (void *)CFBundleGetFunctionPointerForName(thinkGearBundle,CFSTR("TG_ReadPackets"));
        //TG_GetValue = (void *)CFBundleGetFunctionPointerForName(thinkGearBundle,CFSTR("TG_GetValue"));
        //TG_Disconnect = (void *)CFBundleGetFunctionPointerForName(thinkGearBundle,CFSTR("TG_Disconnect"));
        //TG_FreeConnection = (void *)CFBundleGetFunctionPointerForName(thinkGearBundle,CFSTR("TG_FreeConnection"));
        
        //TG_SetDataLog = (void *)CFBundleGetFunctionPointerForName(thinkGearBundle,CFSTR("TG_SetDataLog"));
    }
    
}

MindWave::~MindWave()
{
    CFRelease(mThinkGearBundle);
}
