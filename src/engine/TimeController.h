//
//  TimeController.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-11.
//
//

#pragma once

#include "AudioInputHandler.h"
#include "EaseCurveSelector.h"
#include "Interface.h"

#include "cinder/Timeline.h"

#include <vector>
#include <boost/signals2.hpp>


class TimeController
{
public:
    typedef boost::signals2::signal<void()> TriggerSignal;

public:
    TimeController();
    virtual ~TimeController();
    
    void setupInterface( Interface* interface, const std::string& name, const int midiChannel =-1, const int midiNote =-1 );
    void reset();
    void update(double dt);

    TriggerSignal& getTickSignal()              { return mTickSignal; }
    double getElapsedSeconds() const            { return mElapsedSeconds; }
    double getDelta() const                     { return mDelta; }
    float getTimeScale() const                  { return mTimeScale; }
    bool getTick()                              { return mTick; }
    void setTimeScale(float scale)              { mTimeScale = scale; mTargetTimeScale = scale; }
    void setTimeScaleMultiplier(float scale)    { mTimeScaleMultiplier = scale; mTargetTimeScaleMultiplier = scale; }
    void setTickDuration(float seconds)         { mTickDuration = seconds; }
    
    bool onTargetTimescaleChanged();
    
    float getTransitionTime() const { return mTransitionTime; }
    
public:
    float mMinTimeScale;
    float mMaxTimeScale;
    float mMinTimeScaleMultiplier;
    float mMaxTimeScaleMultiplier;
    
private:
    double mElapsedSeconds;
    double mDelta;
    float mTargetTimeScale;
    ci::Anim<float> mTimeScale;
    float mTargetTimeScaleMultiplier;
    ci::Anim<float> mTimeScaleMultiplier;
    float mTransitionTime;
    
    // tick
    bool mTick;
    double mTickTimer;
    float mTickDuration;
    TriggerSignal mTickSignal;
    
};