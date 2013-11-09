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
TS_SHADERS_ENTRY( "Simplicity", "simplicity_frag.glsl", SHADER_SIMPLICITY ) \
TS_SHADERS_ENTRY( "Contour", "contour_tex_frag.glsl", SHADER_NOISE ) \
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
    struct tNoiseParams
    {
        float       mDisplacementSpeed;
        ci::Vec3f   mNoiseScale;
        float       mLevels;
        float       mEdgeThickness;
        float       mBrightness;
    };
    tNoiseParams        mNoiseParams;
    
    struct tCellsParams
    {
        bool mHighlightAudioResponse;
        float mZoom; //
        float mHighlight; // 6
        
        enum { CELLS_NUM_LAYERS = 7 };
        
        float mTimeStep[CELLS_NUM_LAYERS];
        float mFrequency[CELLS_NUM_LAYERS];
        float mTime[CELLS_NUM_LAYERS];
        
        float mIntensity;
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
    
    struct tSimplicityParams
    {
        ci::Vec3f           mColorScale;
        int                 mRedPower;
        int                 mGreenPower;
        int                 mBluePower;
        float               mStrengthFactor;
        float               mStrengthMin;
        float               mStrengthConst;
        int                 mIterations;
        float               mAccumPower;
        ci::Vec3f           mMagnitude;
        float               mFieldScale;
        float               mFieldSubtract;
        float               mTimeScale;
        ci::Vec3f           mPanSpeed;
        ci::Vec3f           mUVOffset;
        float               mUVScale;
    };
    tSimplicityParams mSimplicityParams;
};

