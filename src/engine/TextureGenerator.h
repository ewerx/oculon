//
//  TextureGenerator.h
//  Oculon
//
//  Created by Ehsan Rezaie on 11/22/2013.
//
//

#pragma once

#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"

#include "Interface.h"

class TextureGenerator
{
public:
    TextureGenerator();
    virtual ~TextureGenerator();
    
    void setup(const int width, const int height);
    void update(double dt);
    
    ci::gl::Texture getTexture()        { return mFbo.getTexture(); }
    void bindTexture(int textureUnit)   { mFbo.bindTexture(textureUnit); }
    void unbindTexture()                { mFbo.unbindTexture(); }
    
    // interface
    virtual void setupInterface( Interface* interface, const std::string& name ) {};
    virtual void initShader() = 0;
    virtual void configShader(double dt) = 0;
    
protected:
    double mElapsedTime;
    float mTimeScale;
    
    ci::gl::Fbo mFbo;
    ci::gl::GlslProg mShader;
    
};