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
    mSpinRate = Vec3f::yAxis()*0.01f;
    mSpinDistance = 100.0f;
    mTargetSpinDistance = mSpinDistance;
    mTransitionTime = 1.0f;
    mSpinUp = Vec3f::yAxis();
    mCam.lookAt(Vec3f(0.0f,0.0f,-mSpinDistance()), Vec3f::zero(), mSpinUp);
    mCam.setPerspective(60.0f, aspectRatio, 0.0001f, 10000.0f);
    mOscillateDistance = 0.0f;
    mTime = 0.0f;
}

SpinCam::~SpinCam()
{
}

void SpinCam::setupInterface(Interface *interface, const std::string &name)
{
    mInterfacePanel = interface->gui()->addPanel();
    interface->gui()->addLabel("spin cam");
    interface->addParam(CreateFloatParam("spin_dist", &mTargetSpinDistance)
                        .minValue(0.01f)
                        .maxValue(800.f))->registerCallback(this, &SpinCam::onSpinDistanceChanged);
    interface->addParam(CreateFloatParam("oscillate", &mOscillateDistance)
                        .minValue(0.0f)
                        .maxValue(1.0f));
    interface->addParam(CreateFloatParam("transition", &mTransitionTime)
                        .minValue(0.0f)
                        .maxValue(300.0f));
    interface->addParam(CreateVec3fParam("spin_axis", &mSpinRate, Vec3f::one()*-0.5f, Vec3f::one()*0.5f));
    interface->addParam(CreateVec3fParam("spin_up", &mSpinUp, Vec3f::zero(), Vec3f::one()));
}

void SpinCam::update(double dt)
{
    float dist = mSpinDistance();
    
    if (mOscillateDistance > 0.0f) {
        float range = mOscillateDistance * dist;
        dist += range * sin(mTime);
        mTime += dt;
    }
    
    mSpinQuat *= Quatf(Vec3f::xAxis(), dt*mSpinRate.x);
    mSpinQuat *= Quatf(Vec3f::yAxis(), dt*mSpinRate.y);
    mSpinQuat *= Quatf(Vec3f::zAxis(), dt*mSpinRate.z);
    Vec3f pos = Vec3f::xAxis() * mSpinQuat;
    pos.normalize();
    pos *= dist;
    mCam.lookAt(pos, Vec3f::zero(), mSpinUp);
}

bool SpinCam::onSpinDistanceChanged()
{
    if (mTargetSpinDistance != mSpinDistance() )
    {
        timeline().apply( &mSpinDistance, mTargetSpinDistance, mTransitionTime, EaseOutQuad() );
    }
    
    return true;
}