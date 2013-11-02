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
    
    // global params
    ci::ColorAf         mColor1;
    ci::ColorAf         mColor2;
    float               mTimeScale;
    double              mElapsedTime;
    
    float               mAudioResponseFreqMin;
    float               mAudioResponseFreqMax;
    
    // color maps
    enum { MAX_COLORMAPS = 4 };
    ci::gl::Texture     mColorMapTexture[MAX_COLORMAPS];
    int                 mColorMapIndex;
    
    // shaders
#define TS_SHADERS_TUPLE \
TS_SHADERS_ENTRY( "Cells", "cells_frag.glsl", SHADER_CELLS ) \
TS_SHADERS_ENTRY( "Kali", "kifs_frag.glsl", SHADER_KALI ) \
TS_SHADERS_ENTRY( "MetaHex", "metahex_frag.glsl", SHADER_METAHEX ) \
TS_SHADERS_ENTRY( "Retina", "retina_frag.glsl", SHADER_RETINA ) \
//end tuple
    
    enum eShaderType
    {
#define TS_SHADERS_ENTRY( nam, glsl, enm ) \
enm,
        TS_SHADERS_TUPLE
#undef  TS_SHADERS_ENTRY
        
        SHADERS_COUNT
    };
    eShaderType   mShaderType;
    
    std::vector<ci::gl::GlslProg> mShaders;
    
    // rendering
    bool mDrawOnSphere;
    ci::gl::Fbo mShaderFbo;
    
    // shader params
    struct tCellsParams
    {
        bool mHighlightAudioResponse;
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
    
    struct tKaliParams
    {
        int iterations;
        float scale;
        ci::Vec2f fold;
        ci::Vec2f translate;
        float zoom;
        float brightness;
        float saturation;
        float texturescale;
        
        float rotspeed;
        
        float colspeed;
        
        float antialias;
    };
    tKaliParams mKaliParams;
    
    struct tMetaHexParams
    {
        float mSpeed;
        float mLightSpeed;
        int mNumObjects;
        int mRenderSteps;
        int mQuality;
    };
    tMetaHexParams mMetaHexParams;
};

