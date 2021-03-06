//
//  LinesRenderer.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2/22/2014.
//
//

#pragma once

#include "ParticleRenderer.h"
#include "AudioInputHandler.h"
#include "cinder/gl/Texture.h"
#include "TextureSelector.h"

class LinesRenderer : public ParticleRenderer
{
public:
    LinesRenderer();
    virtual ~LinesRenderer();
    
    // from ParticleRenderer
    virtual void setup(int fboSize);
    virtual void setupInterface( Interface* interface, const std::string& prefix );
    virtual void draw( PingPongFbo& particlesFbo, const ci::Vec2i& screenSize, const ci::Camera& cam, AudioInputHandler& audioInputHandler );
    
protected:
    // params
    float               mLineWidth;
    bool                mAudioReactive;
    float               mAlphaGain;
    ci::ColorAf         mColor;
    TextureSelector     mColorMaps;
};