//
//  DustRenderer.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-04-02.
//
//

#pragma once

#include "ParticleRenderer.h"
#include "AudioInputHandler.h"
#include "cinder/gl/Texture.h"

class DustRenderer : public ParticleRenderer
{
public:
    DustRenderer();
    virtual ~DustRenderer();
    
    // from ParticleRenderer
    virtual void setup(int fboSize);
    virtual void setupInterface( Interface* interface, const std::string& name );
    virtual void draw( PingPongFbo& particlesFbo, const ci::Vec2i& screenSize, const ci::Camera& cam, AudioInputHandler& audioInputHandler, float gain );
    
protected:
    virtual void preRender();
    virtual void postRender();
    
protected:
    // params
    float               mPointSize;
    bool                mAudioReactive;
    ci::gl::Texture     mColorMapTex;
    ci::gl::Texture     mSpriteTex;
    ci::ColorAf         mColor;
};