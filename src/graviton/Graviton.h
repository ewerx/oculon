//
// Graviton.h
// Oculon
//
// Created by Ehsan on 11-11-26.
// Copyright 2011 ewerx. All rights reserved.
//

#pragma once

#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/Timeline.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Bspline.h"
#include <vector>

#include "Scene.h"
#include "PingPongFbo.h"
#include "MotionBlurRenderer.h"
#include "AudioInputHandler.h"


class Graviton : public Scene
{
public:
    Graviton();
    virtual ~Graviton();
    
    // inherited from Scene
    void setup();
    void reset();
    void resize();
    void update(double dt);
    void draw();
    void drawDebug();
    bool handleKeyDown(const ci::app::KeyEvent& keyEvent);
    void handleMouseDown( const ci::app::MouseEvent& mouseEvent );
	void handleMouseUp( const ci::app::MouseEvent& event);
	void handleMouseDrag( const ci::app::MouseEvent& event );
    const ci::Camera& getCamera();
    
protected:// from Scene
    void setupInterface();
    void setupDebugInterface();
    
protected:
    ci::Surface32f generatePositionSurface();
    ci::Surface32f generateVelocitySurface();

private:
    enum
    {
        kStep =                 512,
        kNumParticles =         (kStep * kStep)
    };
    
    // INITIAL FORMATION
#define GRAVITON_FORMATION_TUPLE \
GRAVITON_FORMATION_ENTRY( "Cube", FORMATION_CUBE ) \
GRAVITON_FORMATION_ENTRY( "Sphere", FORMATION_SPHERE ) \
GRAVITON_FORMATION_ENTRY( "Shell", FORMATION_SPHERE_SHELL ) \
GRAVITON_FORMATION_ENTRY( "Disc", FORMATION_DISC ) \
GRAVITON_FORMATION_ENTRY( "Galaxy", FORMATION_GALAXY ) \
// end tuple
    
    enum eFormation
    {
#define GRAVITON_FORMATION_ENTRY( nam, enm ) \
enm,
        GRAVITON_FORMATION_TUPLE
#undef  GRAVITON_FORMATION_ENTRY
        
        FORMATION_COUNT
    };
    
    // NODE FORMATION
#define GRAVITON_NODE_FORMATION_TUPLE \
GRAVITON_NODE_FORMATION_ENTRY( "None", NODE_FORMATION_NONE ) \
GRAVITON_NODE_FORMATION_ENTRY( "Symmetry", NODE_FORMATION_SYMMETRY ) \
GRAVITON_NODE_FORMATION_ENTRY( "Blackhole", NODE_FORMATION_BLACKHOLE_STAR ) \
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
        
        void applyGravityFrom(tGravityNode &other);
        
        ci::Vec3f   mPos;
        ci::Vec3f   mVel;
        float   mMass;
    };
    
    eFormation              mInitialFormation;
    float                   mFormationRadius;
    
    eNodeFormation              mGravityNodeFormation;
    std::vector<tGravityNode>    mGravityNodes;
    
    bool                    mIsMousePressed;
    ci::Vec2f               mMousePos;
    
    // simulation params
    float mTimeStep;
    float mDamping;
    float mGravity;
    float mEps;
    int32_t mNumNodes;
    uint32_t mFlags;
    uint32_t mNumParticles;
    
    
    // particle system
    PingPongFbo mParticlesFbo;
    ci::gl::VboMesh mVboMesh;
    ci::gl::GlslProg mParticlesShader;
    ci::gl::GlslProg mDisplacementShader;
    ci::gl::GlslProg mFormationShader;
    ci::Vec3f mAttractor1;
    ci::Vec3f mAttractor2;
    ci::Vec3f mAttractor3;

    // rendering
    GLuint              mVbo[2]; // pos and color VBOs
    ci::gl::Texture     mParticleTexture1;
    ci::gl::Texture     mParticleTexture2;
    float				mPointSize;
    bool				mEnableBlending;
    bool				mAdditiveBlending;
    bool                mUseImageForPoints;
    
    bool                mEnableGravityNodes;
    bool                mConstrainParticles;
    
    ci::CameraPersp     mCam;
    
    // camera
#define GRAVITON_CAMTYPE_TUPLE \
GRAVITON_CAMTYPE_ENTRY( "Manual", CAM_MANUAL ) \
GRAVITON_CAMTYPE_ENTRY( "Orbiter", CAM_ORBITER ) \
GRAVITON_CAMTYPE_ENTRY( "Spiral", CAM_SPIRAL ) \
GRAVITON_CAMTYPE_ENTRY( "Spline", CAM_SPLINE ) \
//end tuple
    
    enum eCamType
    {
#define GRAVITON_CAMTYPE_ENTRY( nam, enm ) \
enm,
        GRAVITON_CAMTYPE_TUPLE
#undef  GRAVITON_CAMTYPE_ENTRY
        
        CAM_COUNT
    };
    eCamType                    mCamType;

    float               mCamRadius;
    double              mCamAngle;
    double              mCamLateralPosition;
    float               mCamTurnRate;
    float               mCamTranslateRate;
    float               mCamMaxDistance;
    ci::Vec3f           mCamTarget;
    ci::Quatf           mCamRotation;
    ci::BSpline3f       mCamSpline;
    float               mCamSplineValue;
    ci::Vec3f           mCamLastPos;
    ci::ColorAf         mColorScale;
    
    MotionBlurRenderer  mMotionBlurRenderer;
    
    AudioInputHandler   mAudioInputHandler;

private:
    void setupPingPongFbo();
    void setupVBO();
    void initParticles();
    
    void resetGravityNodes(const eNodeFormation formation);
    void computeAttractorPosition();
    void updateGravityNodes(const double dt);
    
    void updateAudioResponse();
    void updateCamera(const double dt);
        
    void preRender();
    void drawParticles();
    
    bool setupCameraSpline();
    void drawCamSpline();
    
     
private:
    
    
};

