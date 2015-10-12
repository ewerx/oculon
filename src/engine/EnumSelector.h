//
//  EnumSelector.h
//  Oculon1
//
//  Created by Ehsan Rezaie on 2015-05-20.
//
//

#pragma once

#include <vector>
#include "FragShader.h"

class EnumSelector
{
public:
    EnumSelector() : mIndex(0)
    {}
    
    /// return current index
    int operator()() const              { return mIndex; }
    
    /// return current name
    const std::string name() const {
        if (mIndex < mNames.size()) {
            return mNames[mIndex];
        }
        return "none";
    }
    
    void addValue(const std::string& name) {
        mNames.push_back(name);
    }
    
public:
    std::vector<std::string> mNames;
    int mIndex;
};

class ShaderSelector : public EnumSelector
{
public:
    ShaderSelector(): EnumSelector() {};
    void setupInterface(Interface *interface, const std::string& name);
    
    void addShader(const std::string& name, const std::string& fragShader) {
        mShaders.push_back( new FragShader(name, fragShader) );
        addValue(name);
    }

    ci::gl::GlslProg getSelectedShader() const {
        if (mIndex < mShaders.size()) {
            return mShaders[mIndex]->getShader();
        } else {
            return ci::gl::GlslProg();
        }
    }
public:
    std::vector<FragShader*> mShaders;
};