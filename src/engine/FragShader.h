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
#include "NamedObject.h"

class Interface;

/// FragShader base
class FragShader : public oculon::NamedObject<FragShader>
{
public:
    FragShader(const std::string& name, const std::string& fragShader)
    : NamedObject(name)
    {
        mShader = Utils::loadFragShader( fragShader );
    }
    virtual ~FragShader() {}
    
    virtual void setupInterface( Interface* interface, const std::string& prefix ) {}
    virtual void setCustomParams( AudioInputHandler& audioInputHandler ) {}
    virtual void update(double dt) {}
    
    ci::gl::GlslProg& getShader()       { return mShader; }
    
protected:
    ci::gl::GlslProg mShader;
};
