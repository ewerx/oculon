//
//  ParticleController.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2/11/2014.
//
//

#pragma once

#include "PingPongFbo.h"

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
    
    void setup();
    void update(double dt);
    void draw();
    void drawDebug();
    
public:
    
private:
    PingPongFbo mParticlesFbo;
    ci::gl::VboMesh mVboMesh;
    ci::gl::GlslProg mSimulationShader;
    ci::gl::GlslProg mRenderShader;
    
    int mFboSize;
    int mNumParticles;
    
    
};