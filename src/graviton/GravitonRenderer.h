//
//  GravitonRenderer.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2/22/2014.
//
//

#pragma once

#include "ParticleRenderer.h"
#include "AudioInputHandler.h"

class GravitonRenderer : public ParticleRenderer
{
public:
    GravitonRenderer();
    virtual ~GravitonRenderer();
    
    // from ParticleRenderer
    virtual void setup(int fboSize);
    virtual void draw( PingPongFbo& particlesFbo, const ci::Vec2i& screenSize, const ci::Camera& cam, AudioInputHandler& audioInputHandler, float gain );
    
protected:
    virtual void preRender();
    virtual void postRender();
    void drawParticles();
    
protected:
    // params
    ci::gl::Texture     mParticleTexture1;
    ci::gl::Texture     mParticleTexture2;
    float				mPointSize;
    bool				mAdditiveBlending;
    bool                mUseImageForPoints;
    ci::ColorAf         mColor;
};