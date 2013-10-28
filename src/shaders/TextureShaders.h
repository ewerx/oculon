//
//  TextureShaders.h
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

#pragma once

#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "Scene.h"
#include "MotionBlurRenderer.h"
#include "GridRenderer.h"
#include "AudioInputHandler.h"

#include "OscMessage.h"

//
// Audio input tests
//
class TextureShaders : public Scene
{
public:
    TextureShaders();
    virtual ~TextureShaders();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    
    // new
    //ci::gl::Texture getCurrentTexture() const { return mShaders[mShaderType].getTexture(); }
    
protected:// from Scene
    void setupInterface();
    //void setupDebugInterface();
    
private:
    void setupShaders();
    
    void drawScene();
    
    void shaderPreDraw();
    void drawShaderOutput();
    void shaderPostDraw();
    
private:

    //ci::gl::GlslProg    mShader;
    
    // audio
    //AudioInputHandler   mAudioInputHandler;
    
    // params
    ci::ColorAf         mColor1;
    ci::ColorAf         mColor2;
    float               mTimeScale;
    double              mElapsedTime;
    
    bool                mHighlightAudioResponse;
    float               mAudioResponseFreqMin;
    float               mAudioResponseFreqMax;
    
#define SHADERS_TUPLE \
SHADERS_ENTRY( "Cells", "cells_frag.glsl", SHADER_CELLS ) \
//end tuple
    
    enum eShaderType
    {
#define SHADERS_ENTRY( nam, glsl, enm ) \
enm,
        SHADERS_TUPLE
#undef  SHADERS_ENTRY
        
        SHADERS_COUNT
    };
    eShaderType   mShaderType;
    
    struct tCellsParams
    {
        float mCellSize; //
        float mHighlight; // 6
        float mTimeStep1;
        float mTimeStep2;
        float mTimeStep3;
        float mTimeStep4;
        float mTimeStep5;
        float mTimeStep6;
        float mTimeStep7;
    };
    tCellsParams    mCellsParams;
    
    std::vector<ci::gl::GlslProg> mShaders;
    
    bool mDrawOnSphere;
    ci::gl::Fbo mShaderFbo;
};

