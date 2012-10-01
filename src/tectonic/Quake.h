/*
 *  Quake.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#pragma once

#include "Entity.h"
#include "TextEntity.h"

#include "cinder/Timeline.h"

using namespace std;

class Tectonic;
class QuakeEvent;

class Quake : public Entity<float>
{
public:
    Quake(Scene* scene, const QuakeEvent* data, const int mapOffset);
    virtual ~Quake();
    
    // inherited from Entity
    virtual void setup();
    virtual void update(double dt);
    virtual void draw();
    
    // new methods
    void draw( const ci::Color& markerColor, const float ringsMultiplier, const float radiusMultiplier );
    void setEvent( const QuakeEvent* event, const int mapOffset );
    const QuakeEvent*   getEventData() const    { return mEventData; }
    
    void trigger(const float duration);
    void endTrigger();
    bool isTriggered() const                    { return( STATE_TRIGGERED == mState ); }
    
    void drawLabel();
    
protected:
    void updateLabel();
    
protected:
    const QuakeEvent*    mEventData;
    
    TextEntity mLabel;
    bool mShowLabel;
    bool mShowLatLongLines;
    
    enum eState
    {
        STATE_INVALID,
        STATE_IDLE,
        STATE_TRIGGERED
    };
    eState mState;
    
    ci::Anim<float> mGridLinesAlpha;
    ci::Anim<float> mMarkerAlpha;
    ci::Anim<float> mMarkerSize;
};
