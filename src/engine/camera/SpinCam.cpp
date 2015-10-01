//
//  SpinCam.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-10-25.
//
//

#include "SpinCam.h"
#include "cinder/Vector.h"

using namespace ci;
using namespace ci::app;

SpinCam::SpinCam(const float aspectRatio)
: SceneCam("spin", aspectRatio)
{
    mCam.lookAt(Vec3f(0.0f,0.0f,-420.0f), Vec3f(0.0f,0.0f,0.0f), Vec3f(0.0f,1.0f,0.0f));
    mCam.setPerspective(60.0f, aspectRatio, 0.0001f, 10000.0f);
    mSpinRate = Vec3f::yAxis()*0.01f;
    mSpinDistance = 100.0f;
    mTargetSpinDistance = mSpinDistance;
    mTransitionTime = 1.0f;
    mSpinUp = Vec3f::yAxis();
}

SpinCam::~SpinCam()
{
}

void SpinCam::setupInterface(Interface *interface, const std::string &name)
{
    mInterfacePanel = interface->gui()->addPanel();
    interface->gui()->addLabel("spin cam");
    //    interface->addParam(CreateFloatParam("spin_rate", &mSpinRate)
    //                        .maxValue(10.0f));
    interface->addParam(CreateFloatParam("spin_dist", &mTargetSpinDistance)
                        .maxValue(1000.f))->registerCallback(this, &SpinCam::onSpinDistanceChanged);
    interface->addParam(CreateFloatParam("transition", &mTransitionTime)
                        .minValue(0.0f)
                        .maxValue(300.0f));
    interface->addParam(CreateVec3fParam("spin_axis", &mSpinRate, Vec3f::one()*-0.5f, Vec3f::one()*0.5f));
    interface->addParam(CreateVec3fParam("spin_up", &mSpinUp, Vec3f::zero(), Vec3f::one()));
}

void SpinCam::update(double dt)
{
    mSpinQuat *= Quatf(Vec3f::xAxis(), dt*mSpinRate.x);
    mSpinQuat *= Quatf(Vec3f::yAxis(), dt*mSpinRate.y);
    mSpinQuat *= Quatf(Vec3f::zAxis(), dt*mSpinRate.z);
    Vec3f pos = Vec3f::one() * mSpinQuat;
    pos.normalize();
    pos *= mSpinDistance();
    mCam.lookAt(pos, Vec3f::zero(), mSpinUp);
}

bool SpinCam::onSpinDistanceChanged()
{
    if (mTargetSpinDistance != mSpinDistance() )
    {
        timeline().apply( &mSpinDistance, mTargetSpinDistance, mTransitionTime, EaseOutExpo() );
    }
    
    return true;
}