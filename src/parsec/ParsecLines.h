//
//  ParsecLines.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2/14/2014.
//
//

#pragma once

#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"

#include "ParsecStars.h"

#include "PingPongFbo.h"

class ParsecLines
{
public:
    ParsecLines();
    ~ParsecLines();
    
    void setup(std::vector<ParsecStars::Star>& stars);
    void update();
    void draw();
    void preRender();
    void postRender();
    
private:
    PingPongFbo         mParticlesFbo;
    int                 mFboSize;
    int                 mNumParticles;
    
    ci::gl::GlslProg	mRenderShader;
    ci::gl::GlslProg	mSimulationShader;
    ci::gl::VboMesh		mVboMesh;
    
    ci::gl::Texture     mInitialPosTex;
    ci::gl::Texture     mInitialVelTex;
};

    
    
