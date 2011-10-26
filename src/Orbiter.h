/*
 *  Orbiter.h
 *  OculonProto
 *
 *  Created by Ehsan on 11-10-19.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __ORBITER_H__
#define __ORBITER_H__

#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include <vector>

#include "Scene.h"
#include "Body.h"

using namespace ci;
using std::vector;

//
// An orbital simulator
//
class Orbiter : public Scene
{
public:
    Orbiter();
    virtual ~Orbiter();
    
    // inherited from Scene
    void setup();
    void setupParams(params::InterfaceGl& params);
    void reset();
    void update(double dt);
    void draw();
    
private:
    void updateAudioResponse();
    void updateTimeDisplay();
    
    Vec3f           mLightDirection;
    
    vector<Body>    mBodies;
    
    double          mElapsedTime;
    double          mTimeScale;
    double          mDrawScale;
    double          mGravityConstant;
    
    static GLfloat no_mat[];
    static GLfloat mat_ambient[];
    static GLfloat mat_diffuse[];
    static GLfloat mat_specular[];
    static GLfloat mat_emission[];
    static GLfloat mat_shininess[];
    static GLfloat no_shininess[];
    
};



#endif // __ORBITER_H__
