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
#include "MotionBlurRenderer.h"

#include "cinder/Cinder.h"
#include "cinder/audio/Input.h"
#include "cinder/Timeline.h"
#include "cinder/gl/Texture.h"

#include <vector>

class Quake;
class QuakeData;

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
    void initQuakes();
    void clearQuakes();
    
private:
    QuakeData* mData;
    typedef std::vector<Quake*> QuakeList;
    QuakeList mQuakes;
    
    // map
    ci::gl::Texture mEarthDiffuse;
    
    int mCurrentIndex;
};

#endif // __Quaker_H__
