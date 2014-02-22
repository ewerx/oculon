//
//  ParticleController.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2/11/2014.
//
//

#pragma once

#include "PingPongFbo.h"
#include "ParticleBehavior.h"
#include "ParticleRenderer.h"

#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"

#include <vector>


class ParticleController
{
public:
    ParticleController();
    virtual ~ParticleController();
    
    void setup(int fboSize = 256);
    void update(double dt);
    void draw(const ci::Camera& cam);
    void drawDebug();
    
public:
    void addBehavior(ParticleBehavior* behavior) { mBehaviors.push_back(behavior); }
    
private:
    void setupFBO();
    void setupVBO();
    
    void updateSimulation(double dt);
    void render();
    
private:
    PingPongFbo mParticlesFbo;
//    ci::gl::VboMesh mVboMesh;
//    ci::gl::GlslProg mRenderShader;
    
    std::vector<ParticleBehavior*> mBehaviors;
    int mCurrentBehavior;
    
    std::vector<ParticleRenderer*> mRenderers;
    int mCurrentRenderer;
    
    int mFboSize;
    int mNumParticles;
    
};