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
    virtual void setupInterface( Interface* interface, const std::string& prefix );
    virtual void draw( PingPongFbo& particlesFbo, const ci::Vec2i& screenSize, const ci::Camera& cam, AudioInputHandler& audioInputHandler );
    
protected:
    virtual void preRender();
    virtual void postRender();
    
protected:
    // textures
    typedef std::pair<std::string, ci::gl::Texture> tNamedTexture;
    std::vector<tNamedTexture> mPointTextures;
    int mCurPointTexture;
    
    // params
    float				mPointSize;
    bool				mAdditiveBlending;
    ci::ColorAf         mColor;
    bool                mAudioReactive;
    float               mAlphaGain;
};