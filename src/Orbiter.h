/*
 *  Orbiter.h
 *  Oculon
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
#include "MidiMap.h"

using namespace ci;
using std::vector;

class Body;
class Sun;

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
    void resize();
    void update(double dt);
    void draw();
    bool handleKeyDown(const KeyEvent& keyEvent);
    
    // midi callbacks
    void handleGravityChange(MidiEvent midiEvent);
    void handleTimeScaleChange(MidiEvent midiEvent);
    
private:
    void setupMidiMapping();
    
    void updateAudioResponse();
    void updateTimeDisplay();
    
private:
    Vec3f           mLightDirection;
    
    typedef vector<Body*> BodyList;
    BodyList        mBodies;
    
    Sun*                mSun;
    int                 mFollowTargetIndex;
    Body*               mFollowTarget;
    bool                mIsFollowCameraEnabled;
    
    double          mElapsedTime;
    double          mTimeScale;
    double          mDrawScale;
    double          mGravityConstant;
    
    MidiMap         mMidiMap;
    
public:
    static double   sDefaultTimeScale;
    static double   sDefaultGravityConstant;
    static float    sMaxRadiusMultiplier;
    static int      sNumFramesToAvgFft;
};

#endif // __ORBITER_H__
