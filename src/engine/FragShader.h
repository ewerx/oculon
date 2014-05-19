//
//  FragShader.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-18.
//
//

#pragma once

#include "cinder/gl/GlslProg.h"
#include "Utils.h"

class Interface;

class FragShader
{
public:
    FragShader(const std::string& name, const std::string& fragShader)
    : mName(name)
    {
        mShader = Utils::loadFragShader( fragShader );
    }
    virtual ~FragShader() {}
    
    virtual void setupInterface( Interface* interface, const std::string& name ) {};
    virtual void setCustomParams( AudioInputHandler& audioInputHandler ) {}
    
    const std::string& getName()        { return mName; }
    ci::gl::GlslProg& getShader()       { return mShader; }
    
protected:
    std::string mName;
    ci::gl::GlslProg mShader;
};
