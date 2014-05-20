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
    
    bool resetGravityNodes();
    void updateGravityNodes(const double dt);

private:
    // NODE FORMATION
#define GRAVITON_NODE_FORMATION_TUPLE \
GRAVITON_NODE_FORMATION_ENTRY( "Static", NODE_FORMATION_STATIC ) \
GRAVITON_NODE_FORMATION_ENTRY( "Random", NODE_FORMATION_RANDOM ) \
GRAVITON_NODE_FORMATION_ENTRY( "Mirror", NODE_FORMATION_MIRROR ) \
GRAVITON_NODE_FORMATION_ENTRY( "Spin", NODE_FORMATION_SPIN ) \
// end tuple
    
    enum eNodeFormation
    {
#define GRAVITON_NODE_FORMATION_ENTRY( nam, enm ) \
enm,
        GRAVITON_NODE_FORMATION_TUPLE
#undef  GRAVITON_NODE_FORMATION_ENTRY
        
        NODE_FORMATION_COUNT
    };
    
    struct tGravityNode
    {
        tGravityNode(const ci::Vec3f& pos, const ci::Vec3f& vel, const float mass)
        : mPos(pos), mVel(vel), mMass(mass)
        {}
        
        void applyGravityFrom(tGravityNode &other, double dt);
        
        ci::Vec3f   mPos;
        ci::Vec3f   mVel;
        float       mMass;
    };
    
    float                   mFormationRadius;
    
    eNodeFormation                  mGravityNodeFormation;
    std::vector<tGravityNode>       mGravityNodes;
    
    
    // simulation params
//    float mTimeStep;
    float mDamping;
    float mGravity;
    bool mReset;
    bool mAudioGravity;
    bool mAudioContainer;
    bool mAudioMirror;
    bool mRandomMirror;
    float mEps;
    float mConstraintSphereRadius;
    float mNodeSpeed;
    int32_t mNumNodes;
    
    NodeController mNodeController;
    
    // particle system
    ParticleController mParticleController;
    ci::gl::GlslProg mSimulationShader;
    
    // audio
    AudioInputHandler   mAudioInputHandler;

    // camera
    CameraController    mCameraController;
    
    TimeController      mTimeContoller;
};

