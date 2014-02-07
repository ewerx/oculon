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
#include "PingPongFbo.h"
#include "AudioInputHandler.h"
#include "CameraController.h"
#include "EaseCurveSelector.h"

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
    //void setupDebugInterface();
    
    // callbacks
    bool takeFormation();
    
private:
    void setupFBO();
    void setupVBO();
    
    void initParticles();
    void updateParticles();
    void drawParticles();
    
    void setupDynamicTexture();
    void generateDynamicTexture();
    
    void generateFormationTextures();
    
    ci::gl::Fbo mDynamicTexFbo;
    ci::gl::GlslProg mDynamicTexShader;
    float mNoiseSpeed;
	float mNoiseTheta;
    ci::Vec3f mNoiseScale;
    
private:
    // particle system
    enum
    {
        kBufSize = 128,
        kNumParticles = (kBufSize * kBufSize)
    };
    
    PingPongFbo mParticlesFbo;
    ci::gl::VboMesh mVboMesh;
    ci::gl::GlslProg mSimulationShader;
    ci::gl::GlslProg mRenderShader;
    
	ci::gl::Texture mInitialVelTex;
	ci::gl::Texture mParticleDataTex;
    ci::gl::Texture mNoiseTex;
    
    ci::gl::Texture mColorMapTex;
    ci::ColorAf mColor;
    
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
    float mLineWidth;
    
    bool mUseDynamicTex;
    bool mReset;
    ci::Anim<float> mFormationStep;
    EaseCurveSelector mFormationAnimSelector;
    
    // audio
    AudioInputHandler mAudioInputHandler;
    
    bool mAudioReactive;
    bool mAudioTime;

};


