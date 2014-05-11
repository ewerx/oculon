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
    
    void setupInterface( Interface* interface, const std::string& name );
    void reset();
    void update(double dt);

    TriggerSignal& getTickSignal()      { return mTickSignal; }
    double getElapsedSeconds() const    { return mElapsedSeconds; }
    double getDelta() const             { return mDelta; }
    bool getTick()                      { return mTick; }
    void setTimeScale(float scale)      { mTimeScale = scale; }
    void setTickDuration(float seconds) { mTickDuration = seconds; }
    
private:
    double mElapsedSeconds;
    double mDelta;
    float mTimeScale;
    
    // tick
    bool mTick;
    double mTickTimer;
    float mTickDuration;
    TriggerSignal mTickSignal;
    
};