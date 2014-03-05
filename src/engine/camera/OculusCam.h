//
//  OculusCam.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2/23/2014.
//
//

#pragma once

#include "OculusVR.h"
#include "CameraStereoHMD.h"
#include "cinder/Camera.h"

class Interface;

class OculusCam
{
public:
    OculusCam();
    ~OculusCam();
    
    void setup();
    void update();
    
    void enableStereoLeft() { mCamera.enableStereoLeft(); }
    void enableStereoRight() { mCamera.enableStereoRight(); }
    
    const ci::CameraPersp& getCamera();
    
private:
    ovr::DeviceRef              mOculusVR;
    CameraStereoHMD             mCamera;
    bool                        mStereoRight;
};