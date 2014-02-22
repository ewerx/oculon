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
#include "cinder/gl/Texture.h"


class GravitonRenderer : public ParticleRenderer
{
public:
    GravitonRenderer();
    virtual ~GravitonRenderer();
    
    // from ParticleRenderer
    virtual void setup(int fboSize);
    virtual void setupInterface( Interface* interface, const std::string& name );
    virtual void draw( PingPongFbo& particlesFbo, const ci::Vec2i& screenSize, const ci::Camera& cam, AudioInputHandler& audioInputHandler, float gain );
    
protected:
    virtual void preRender();
    virtual void postRender();
    
protected:
    // params
    ci::gl::Texture     mParticleTexture1;
    ci::gl::Texture     mParticleTexture2;
    float				mPointSize;
    bool				mAdditiveBlending;
    bool                mUseImageForPoints;
    ci::ColorAf         mColor;
};