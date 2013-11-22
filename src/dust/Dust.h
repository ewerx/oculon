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
    //void setupDebugInterface();
    
private:
    void setupFBO();
    void setupVBO();
    
//    std::vector<ci::Surface32f> generateInitialSurfaces();
    
    void initParticles();
    void updateParticles();
    void drawParticles();
    
private:
    // particle system
    enum
    {
        kBufSize = 1024,
        kNumParticles = (kBufSize * kBufSize)
    };
    
    PingPongFbo mParticlesFbo;
    ci::gl::VboMesh mVboMesh;
    ci::gl::GlslProg mSimulationShader;
    ci::gl::GlslProg mRenderShader;
    
    ci::gl::Texture mInitialPosTex;
	ci::gl::Texture mInitialVelTex;
	ci::gl::Texture mParticleDataTex;
    ci::gl::Texture mNoiseTex;
    
    ci::gl::Texture mSpriteTex;
    
    // params
    float mTimeStep;
    float mPointSize;
    float mDecayRate;
    
    //AudioInputHandler   mAudioInputHandler;
    
};


