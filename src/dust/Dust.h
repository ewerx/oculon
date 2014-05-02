//
//  Dust.h
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
#include "DustRenderer.h"
#include "LinesRenderer.h"
#include "SimplexNoiseTexture.h"

//
// Dust
//
class Dust : public Scene
{
public:
    Dust();
    virtual ~Dust();
    
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
    void setupFBO();
    
    ParticleRenderer& getRenderer();
    
    void initParticles();
    void updateParticles();
    void drawParticles();
    
    SimplexNoiseTexture mDynamicTexture;
    
private:
    // particle system
    enum
    {
        kBufSize = 512,
        kNumParticles = (kBufSize * kBufSize)
    };
    
    PingPongFbo mParticlesFbo;
    ci::gl::GlslProg mSimulationShader;
    
    DustRenderer mRenderer;
    LinesRenderer mLinesRenderer;
    bool mAltRenderer;
    
    ci::CameraOrtho mCamera;
    
    ci::gl::Texture mInitialPosTex;
	ci::gl::Texture mInitialVelTex;
    
    // params
    float mTimeStep;
    float mDecayRate;
    
    bool mReset;
    
    AudioInputHandler mAudioInputHandler;
    
    // MUTEK HACKS
    bool mAudioReactive;
    bool mAudioTime;
};


