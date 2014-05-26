//
//  FragShader.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-18.
//
//

#pragma once

#include "AudioInputHandler.h"
#include "cinder/gl/GlslProg.h"
#include "Utils.h"

class Interface;

/// FragShader base
class FragShader
{
public:
    FragShader(const std::string& name, const std::string& fragShader)
    : mName(name)
    {
        mShader = Utils::loadFragShader( fragShader );
    }
    virtual ~FragShader() {}
    
    virtual void setupInterface( Interface* interface, const std::string& prefix ) {}
    virtual void setCustomParams( AudioInputHandler& audioInputHandler ) {}
    virtual void update(double dt) {}
    
    const std::string& getName()        { return mName; }
    ci::gl::GlslProg& getShader()       { return mShader; }
    
protected:
    std::string mName;
    ci::gl::GlslProg mShader;
};
