//
//  ParticleRenderer.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2/20/2014.
//
//

#pragma once

#include "Interface.h"
#include "PingPongFbo.h"
#include "AudioInputHandler.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"

class ParticleRenderer
{
public:
    ParticleRenderer(const std::string& name);
    virtual ~ParticleRenderer();
    
    virtual void setup(int fboSize) = 0;
    virtual void setupInterface( Interface* interface, const std::string& prefix ) {};
    virtual void draw( PingPongFbo& particlesFbo, const ci::Vec2i& screenSize, const ci::Camera& cam, AudioInputHandler& audioInputHandler, float gain );
    
    const std::string& getName() const            { return mName; }
    
protected:
    void setupVBO(int fboSize, GLuint primitiveType);
    
    virtual void preRender() {};
    virtual void postRender() {};
    
protected:
    std::string mName;
    ci::gl::VboMesh mVboMesh;
    ci::gl::GlslProg mShader;
};