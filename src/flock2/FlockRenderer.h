//
//  FlockRenderer.h
//  Oculon
//
//  Created by Ehsan Rezaie on 10/10/2015.
//
//

#pragma once

#include "ParticleRenderer.h"
#include "AudioInputHandler.h"
#include "cinder/gl/Texture.h"

class FlockRenderer : public ParticleRenderer
{
public:
    FlockRenderer();
    virtual ~FlockRenderer();
    
    // from ParticleRenderer
    virtual void setup(int fboSize);
    virtual void setupInterface( Interface* interface, const std::string& prefix );
    virtual void draw( PingPongFbo& particlesFbo, const ci::Vec2i& screenSize, const ci::Camera& cam, AudioInputHandler& audioInputHandler );
    
protected:
    // params
    float               mLineWidth;
    bool                mAudioReactive;
    float               mAlphaGain;
    ci::gl::Texture     mColorMapTex;
    ci::ColorAf         mColor;
    bool                mUseColorMap;
};