//
//  OculusCam.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2/23/2014.
//
//

#include "OculusCam.h"

using namespace ci;

OculusCam::OculusCam()
{
}

OculusCam::~OculusCam()
{
}

void OculusCam::setup()
{
    // Init OVR
    mOculusVR = ovr::Device::create();
    
    // Create Stereo Camera
    mCamera = CameraStereoHMD( 640, 800, mOculusVR ? mOculusVR->getFov() : 125, mOculusVR ? mOculusVR->getEyeToScreenDistance() : 10, 10000.0f );
    mCamera.setEyePoint( Vec3f::zero() );
    mCamera.setWorldUp( Vec3f( 0, 1, 0 ) );
    
    // Make the stereo a bit stronger
    //mCamera.setEyeSeparation( 1.5f );
    
    mStereoRight = false;
}

void OculusCam::update()
{
    Quatf orientation;
    if( mOculusVR )
    {
        orientation = mOculusVR->getOrientation();
    }
    
    mCamera.setOrientation( orientation * Quatf( Vec3f( 0, 1, 0 ), M_PI ) );
}

const CameraPersp& OculusCam::getCamera()
{
    // swap left/right stereo on each call
//    if (mStereoRight)
//    {
//        mCamera.enableStereoRight();
//    }
//    else
//    {
//        mCamera.enableStereoLeft();
//    }
//    mStereoRight = !mStereoRight;
    
    return mCamera;
}