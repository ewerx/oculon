//
//  DomeCam.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-03-24.
//
//

#include "DomeCam.h"

using namespace ci;

DomeCam::DomeCam()
{
}

DomeCam::~DomeCam()
{
}

void DomeCam::setup()
{
    mCam = CameraPersp(1024, 1024, 90.0f);
    mCam.setNearClip(0.01f);
    mCam.setFarClip(200000.0f);
    mCam.lookAt(Vec3f::zero(), Vec3f(0.0f,1.0f,0.0f));
}

void DomeCam::update()
{
}

const CameraPersp DomeCam::getCamera(DomeCam::eOrientation orientation)
{
    CameraPersp cam(mCam);
    
    switch (orientation)
    {
        case CAMERA_LEFT:
            cam.lookAt( mCam.getViewDirection().cross(mCam.getWorldUp()) );
            break;
            
        case CAMERA_RIGHT:
            cam.lookAt( -mCam.getViewDirection().cross(mCam.getWorldUp()) );
            break;
            
        case CAMERA_TOP:
            cam.lookAt( mCam.getWorldUp() );
            break;
            
        case CAMERA_BACK:
            cam.lookAt( -mCam.getViewDirection() );
            break;
            
        case CAMERA_FRONT:
        default:
            break;
    }
    
    return cam;
}