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
    TextureShaders(const std::string& name);
    virtual ~TextureShaders();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    
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
    
    AudioInputHandler   mAudioInputHandler;
    
    float               mAudioResponseFreqMin;
    float               mAudioResponseFreqMax;
    
    // color maps
    enum { MAX_COLORMAPS = 4 };
    ci::gl::Texture     mColorMapTexture[MAX_COLORMAPS];
    int                 mColorMapIndex;
    
    // shaders
#define TS_SHADERS_TUPLE \
TS_SHADERS_ENTRY( "Kali", "kifs_frag.glsl", SHADER_KALI ) \
TS_SHADERS_ENTRY( "Simplicity", "simplicity_frag.glsl", SHADER_SIMPLICITY ) \
TS_SHADERS_ENTRY( "Voronoi", "voronoi_frag.glsl", SHADER_VORONOI ) \
//TS_SHADERS_ENTRY( "Contour", "contour_tex_frag.glsl", SHADER_NOISE ) \
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
    
    // audio respons types
#define AUDIO_RESPONSE_TYPE_TUPLE \
AUDIO_RESPONSE_TYPE_ENTRY( "None", AUDIO_RESPONSE_NONE ) \
AUDIO_RESPONSE_TYPE_ENTRY( "SingleBand", AUDIO_RESPONSE_SINGLE ) \
AUDIO_RESPONSE_TYPE_ENTRY( "MultiBand", AUDIO_RESPONSE_MULTI ) \
//end tuple
    
    enum eAudioRessponseType
    {
#define AUDIO_RESPONSE_TYPE_ENTRY( nam, enm ) \
enm,
        AUDIO_RESPONSE_TYPE_TUPLE
#undef  AUDIO_RESPONSE_TYPE_ENTRY
        
        AUDIO_RESPONSE_TYPE_COUNT
    };
    
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
    
    struct tVoronoiParams
    {
        ci::Vec3f mBorderColor;
        float mSpeed;
        float mZoom;
        float mBorderIn;
        float mBorderOut;
        ci::Vec3f mSeedColor;
        float mSeedSize;
        float mCellLayers;
        ci::Vec3f mCellColor;
        float mCellBorderStrength;
        float mCellBrightness;
        float mDistortion;
        bool mAudioDistortion;
    };
    tVoronoiParams mVoronoiParams;
};

