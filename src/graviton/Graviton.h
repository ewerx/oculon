//
// Graviton.h
// Oculon
//
// Created by Ehsan on 11-11-26.
// Copyright 2011 ewerx. All rights reserved.
//

#pragma once

#include <vector>
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/Timeline.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"
#include "Scene.h"
#include "PingPongFbo.h"
#include "AudioInputHandler.h"
#include "CameraController.h"
#include "EaseCurveSelector.h"
#include "ParticleController.h"
#include "NodeController.h"
#include "TimeController.h"
#include "PlanetObject.h"


/// Graviton
///
class Graviton : public Scene
{
public:
    Graviton();
    virtual ~Graviton();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    const ci::Camera& getCamera();
    
protected:// from Scene
    void setupInterface();
    //void setupDebugInterface();

private:
    void setupParticles(const int bufSize);

private:
    float mFormationRadius;
    
    // simulation params
    float mDamping;
    float mGravity;
    float mGravity2;
    bool mReset;
    bool mAudioGravity;
    bool mAudioContainer;
    bool mAudioMirror;
    bool mRandomMirror;
    bool mSpin;
    float mEps;
    float mConstraintSphereRadius;
    
    NodeController mNodeController;
    
    // particle system
    ParticleController mParticleController;
    ci::gl::GlslProg mSimulationShader;
    
    // audio
    AudioInputHandler   mAudioInputHandler;

    // camera
    CameraController    mCameraController;
    
    TimeController      mTimeController;
    
    PlanetObject        mMoon;
    bool                mDrawMoon;
    
    bool                mGravitySync;
};

