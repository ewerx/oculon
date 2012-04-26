/*
 *  Quaker.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __Quaker_H__
#define __Quaker_H__

#include "Scene.h"
#include "QuakeData.h"
#include "MotionBlurRenderer.h"

#include "cinder/Cinder.h"
#include "cinder/audio/Input.h"
#include "cinder/Timeline.h"


//
// Audio input tests
//
class Quaker : public Scene
{
public:
    Quaker();
    virtual ~Quaker();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    
protected:
    void setupInterface();
    
    
    
private:
    QuakeData* mData;
};

#endif // __Quaker_H__
