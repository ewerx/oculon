//
//  Lines.h
//  Oculon
//
//  Created by Ehsan Rezaie on 11/21/2013.
//
//

#pragma once

#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "Scene.h"
#include "ParticleController.h"
#include "AudioInputHandler.h"
#include "CameraController.h"
#include "EaseCurveSelector.h"
#include "LinesRenderer.h"
#include "GravitonRenderer.h"
#include "SimplexNoiseTexture.h"

//
// Lines
//
class Lines : public Scene
{
public:
    Lines();
    virtual ~Lines();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    const ci::Camera& getCamera();
    
protected:// from Scene
    void setupInterface();
    ////void setupDebugInterface();
    
    // callbacks
    bool takeFormation();
    
private:
    void setupFBO();
    
    void initParticles();
    void updateParticles();
    void drawParticles();
    
    ParticleRenderer& getRenderer();
    
    SimplexNoiseTexture mDynamicTexture;
    
private:
    // particle system
    ParticleController mParticleController;
    
    ci::gl::GlslProg mSimulationShader;
    
    // formations
#define FORMATION_TUPLE \
FORMATION_ENTRY( "Random", FORMATION_RANDOM ) \
FORMATION_ENTRY( "Straight", FORMATION_STRAIGHT ) \
// end tuple
    
    enum eFormation
    {
#define FORMATION_ENTRY( nam, enm ) \
enm,
        FORMATION_TUPLE
#undef  FORMATION_ENTRY
        
        FORMATION_COUNT
    };
    eFormation mFormation;
    ci::gl::Texture mFormationPosTex[FORMATION_COUNT];
    
    // motion
#define MOTION_TUPLE \
MOTION_ENTRY( "Static", MOTION_STATIC ) \
MOTION_ENTRY( "Noise", MOTION_NOISE ) \
MOTION_ENTRY( "Gravity", MOTION_GRAVITY ) \
// end tuple
    
    enum eMotion
    {
#define MOTION_ENTRY( nam, enm ) \
enm,
        MOTION_TUPLE
#undef  MOTION_ENTRY
        
        MOTION_COUNT
    };
    eMotion mMotion;

    // camera
    CameraController mCameraController;
    
    // params
    float mTimeStep;
    
    bool mReset;
    ci::Anim<float> mFormationStep;
    EaseCurveSelector mFormationAnimSelector;
    
    // audio
    AudioInputHandler mAudioInputHandler;
    bool mAudioTime;

    LinesRenderer mRenderer;
    GravitonRenderer mGravitonRenderer;
    bool mAltRenderer;
    
    int mBufSize;
};


