//
//  TimeController.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-11.
//
//

#include "TimeController.h"

#pragma mark - constructor/destructor

TimeController::TimeController()
: mElapsedSeconds(0.0f)
, mDelta(0.0f)
, mTimeScale(1.0f)
, mTimeScaleMultiplier(1.0f)
, mTick(false)
, mTickTimer(0.0f)
, mTickDuration(1.0f)
, mMinTimeScale(0.00001f)
, mMaxTimeScale(1.0f)
, mMinTimeScaleMultiplier(1.0f)
, mMaxTimeScaleMultiplier(10.0f)
{
}

TimeController::~TimeController()
{
}

#pragma mark - setup

void TimeController::reset()
{
    mElapsedSeconds = 0.0f;
    mDelta = 0.0f;
    mTickTimer = 0.0f;
    mTick = false;
}

void TimeController::setupInterface(Interface *interface, const std::string &name)
{
    interface->addParam(CreateFloatParam("timescale", &mTimeScale)
                        .minValue(mMinTimeScale)
                        .maxValue(mMaxTimeScale)
                        .oscReceiver(name));
    interface->addParam(CreateFloatParam("time_multi", &mTimeScale)
                        .minValue(mMinTimeScaleMultiplier)
                        .maxValue(mMaxTimeScaleMultiplier)
                        .oscReceiver(name));
}

#pragma mark - update

void TimeController::update(double dt)
{
    mDelta = dt * mTimeScale * mTimeScaleMultiplier;
    
    mElapsedSeconds += mDelta;
    
    mTickTimer += mDelta;
    mTick = false;
    if (mTickTimer > mTickDuration)
    {
        mTickTimer = 0.0f;
        mTick = true;
        mTickSignal();
    }
}
