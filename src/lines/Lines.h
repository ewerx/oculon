//
//  Lines.h
//  Oculon
//
//  Created by Ehsan Rezaie on 11/21/2013.
//
//

#pragma once

#include "AudioInputHandler.h"
#include "CameraController.h"
#include "DustRenderer.h"
#include "EaseCurveSelector.h"
#include "GravitonRenderer.h"
#include "LinesRenderer.h"
#include "ParticleController.h"
#include "Scene.h"
#include "SimplexNoiseTexture.h"

#include "cinder/Cinder.h"
#include "cinder/gl/GlslProg.h"
#include <vector>

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
    void takeFormation();
    
private:
    void setupParticles(const int bufSize);
    
    SimplexNoiseTexture mDynamicTexture;
    
private:
    // particle system
    ParticleController mParticleController;
    
    ci::gl::GlslProg mSimulationShader;
    
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
};


