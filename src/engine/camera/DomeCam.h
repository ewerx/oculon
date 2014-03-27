//
//  DomeCam.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-03-24.
//
//

#pragma once

#include "cinder/Camera.h"

class Interface;

class DomeCam
{
public:
    enum eOrientation
    {
        CAMERA_TOP,
        CAMERA_LEFT,
        CAMERA_FRONT,
        CAMERA_RIGHT,
        CAMERA_BACK,
        
        CAMERA_COUNT
    };
public:
    DomeCam();
    ~DomeCam();
    
    void setup();
    void update();
    
    const ci::CameraPersp getCamera(eOrientation orientation);
    
private:
    ci::CameraPersp             mCamera[CAMERA_COUNT];
    ci::CameraPersp             mCam;
};