//
//  FractalShaders.h
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

#pragma once

#include <vector>
#include "cinder/Cinder.h"
#include "Scene.h"
#include "AudioInputHandler.h"
#include "FragShader.h"
#include "TimeController.h"

//
//
class FractalShaders : public Scene
{
public:
    FractalShaders();
    virtual ~FractalShaders();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    //void drawDebug();
    
protected:// from Scene
    void setupInterface();
    //void setupDebugInterface();
    
private:
    void drawScene();
    
    void shaderPreDraw();
    void shaderPostDraw();
    
private:
    TimeController      mTimeController;
    
    // global params
    ci::ColorAf         mColor1;
    ci::ColorAf         mColor2;
    
    // textures
    typedef std::pair<std::string, ci::gl::Texture> tNamedTexture;
    
    std::vector<tNamedTexture> mColorMaps;
    int mColorMapIndex;
    
    // shaders
    std::vector<FragShader*> mShaders;
    int mShaderType;
};

