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

using namespace ci;
using namespace std;

class Quaker;
class QuakeEvent;

class Quake : public Entity<float>
{
public:
    Quake(Scene* scene, const QuakeEvent* data);
    virtual ~Quake();
    
    // inherited from Entity
    virtual void setup();
    virtual void update(double dt);
    virtual void draw();
    
    // new methods
    const QuakeEvent*   getEventData() const    { return mEventData; }
    
protected:
    void updateLabel();
    void drawLabel();
    
protected:
    const QuakeEvent*    mEventData;
    
    TextEntity mLabel;
    bool mIsLabelVisible;
};
