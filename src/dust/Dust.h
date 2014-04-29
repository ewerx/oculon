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
    
protected:// from Scene
    void setupInterface();
    ////void setupDebugInterface();
    
private:
    void setupFBO();
//    void setupVBO();
    
//    std::vector<ci::Surface32f> generateInitialSurfaces();
    
    ParticleRenderer& getRenderer();
    
    void initParticles();
    void updateParticles();
    void drawParticles();
    
    void setupDynamicTexture();
    void generateDynamicTexture();
    ci::gl::Fbo mDynamicTexFbo;
    ci::gl::GlslProg mDynamicTexShader;
    float mNoiseSpeed;
	float mNoiseTheta;
    ci::Vec3f mNoiseScale;
    
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
    
    ci::gl::Texture mInitialPosTex;
	ci::gl::Texture mInitialVelTex;
	ci::gl::Texture mParticleDataTex;
    ci::gl::Texture mNoiseTex;
    
    // params
    float mTimeStep;
    float mDecayRate;
    
    bool mUseDynamicTex;
    bool mReset;
    
    AudioInputHandler mAudioInputHandler;
    
    // MUTEK HACKS
    bool mAudioReactive;
    bool mAudioTime;
};


