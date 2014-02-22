//
//  ParticleBehavior.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2/20/2014.
//
//

#pragma once

#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "PingPongFbo.h"
#include "Interface.h"

class ParticleBehavior
{
public:
    ParticleBehavior();
    ~ParticleBehavior();
    
    virtual void setup();
    virtual void setupInterface(Interface &interface, const std::string& name);
    virtual void update(double dt, PingPongFbo &particleFbo);
    
protected:
    ci::gl::GlslProg mShader;
};