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

//int (*TG_GetDV)()	= NULL;	 
//int (*TG_GetNCId)() = NULL;	
//int (*TG_Connect)(int, const char *, int, int) = NULL;


class MindWave
{
public:
    MindWave();
    ~MindWave();
    
    void setup();
    void update();
    
private:
    CFURLRef                mBundleUrl;
    CFBundleRef             mThinkGearBundle;
    
    int (*TG_GetDV)();      // TG_GetDriverVersion
    int (*TG_GetNCId)();    // TG_GetNewConnectionId 
    int (*TG_Connect)(int, const char *, int, int);
};

#endif // __MINDWAVE_H__
