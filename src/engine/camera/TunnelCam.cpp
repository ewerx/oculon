//
//  TunnelCam.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2013-03-29.
//
//

#include "TunnelCam.h"
#include "Interface.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include <vector>

using namespace ci;
using namespace std;

TunnelCam::TunnelCam()
: SceneCam("tunnel", 0.0f) // TODO
{
}

TunnelCam::~TunnelCam()
{
}

void TunnelCam::setup(const Vec3f& startPos, bool loop)
{
    mTarget = Vec3f(0.0f, 1.0f, 0.0f);
    mSpeed = 1.0f;
    mSpeedMultiplier = 1.0f;
    mStartPos = startPos;
    mLoop = loop;
    mTheta = 0;
    mRadius = 200.0f;
    
    mCam.setNearClip(0.0001f);
    mCam.setFarClip(1000.0f);
    
    reset();
}

bool TunnelCam::reset()
{
    mLastPos = mStartPos;
    mCam.lookAt( mTarget*-500.0f, mStartPos );
    return false;
}

void TunnelCam::setupInterface( Interface* interface, const std::string& name)
{
    mInterfacePanel = interface->gui()->addPanel();
    interface->gui()->addLabel("Tunnel Cam");
    interface->addButton(CreateTriggerParam("Reset Tunnel", NULL)
                         .oscReceiver(name,"tunnelcamreset"))->registerCallback( this, &TunnelCam::reset );
    interface->addParam(CreateFloatParam( "Speed", &mSpeed )
                        .minValue(-20.0f)
                        .maxValue(20.0f)
                        .oscReceiver(name, "tunnelcamspeed"));
    interface->addParam(CreateFloatParam( "Speed Multiplier", &mSpeedMultiplier )
                        .minValue(0.0f)
                        .maxValue(10.0f)
                        .oscReceiver(name, "tunnelcamspeedmult"));
}

void TunnelCam::update(double dt)
{
    if (mLoop) {
        mTheta += dt * mSpeed * mSpeedMultiplier;
        Vec3f pos( math<float>::cos( mTheta ) * mRadius,
                                   math<float>::sin( mTheta ) * mRadius,
                                   0.0f
                                   );
        Vec3f dir = (pos - mLastPos);
        mCam.setEyePoint(pos);
        mCam.setViewDirection(dir);
        mLastPos = pos;
    }
    else {
        Vec3f delta = 10.0f * mSpeed * mSpeedMultiplier * dt * mTarget;
        Vec3f pos = mLastPos + delta;
        mCam.setEyePoint(pos);
        mLastPos = pos;
    }
}
