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
    virtual void setupInterface( Interface* interface, const std::string& prefix );
    virtual void draw( PingPongFbo& particlesFbo, const ci::Vec2i& screenSize, const ci::Camera& cam, AudioInputHandler& audioInputHandler );
    
protected:

    
protected:
    // params
    float               mPointSize;
    bool                mAudioReactive;
    bool                mAdditiveBlend;
    ci::ColorAf         mColor;
    
    // textures
    typedef std::pair<std::string, ci::gl::Texture> tNamedTexture;
    std::vector<tNamedTexture> mPointTextures;
    int mCurPointTexture;
    
    std::vector<tNamedTexture> mColorMaps;
    int mCurColorMap;
};