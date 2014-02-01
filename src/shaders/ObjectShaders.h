//
//  ObjectShaders.h
//  Oculon
//
//  Created by Ehsan on 13-11-02.
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
class ObjectShaders : public Scene
{
public:
    ObjectShaders();
    virtual ~ObjectShaders();
    
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
    ci::ColorAf         mColor3;
    float               mTimeScale;
    double              mElapsedTime;
    
    float               mAudioResponseFreqMin;
    float               mAudioResponseFreqMax;
    
    // color maps
    enum { MAX_COLORMAPS = 4 };
    ci::gl::Texture     mColorMapTexture[MAX_COLORMAPS];
    int                 mColorMapIndex;
    
    // noise texture
    ci::gl::Texture     mNoiseTexture;
    
    // shaders
#define OS_SHADERS_TUPLE \
OS_SHADERS_ENTRY( "MetaHex", "metahex_frag.glsl", SHADER_METAHEX ) \
OS_SHADERS_ENTRY( "Retina", "retina_frag.glsl", SHADER_RETINA ) \
OS_SHADERS_ENTRY( "BioFractal", "livingkifs_frag.glsl", SHADER_BIOFRACTAL ) \
OS_SHADERS_ENTRY( "TEST-Clouds", "clouds_frag.glsl", SHADER_CLOUDS ) \
OS_SHADERS_ENTRY( "TEST-Fireball", "fireball_frag.glsl", SHADER_FIREBALL ) \
OS_SHADERS_ENTRY( "OLD-Tilings", "tilings_frag.glsl", SHADER_TILINGS ) \
//end tuple
    
    enum eShaderType
    {
#define OS_SHADERS_ENTRY( nam, glsl, enm ) \
enm,
        OS_SHADERS_TUPLE
#undef  OS_SHADERS_ENTRY
        
        SHADERS_COUNT
    };
    eShaderType   mShaderType;
    
    std::vector<ci::gl::GlslProg> mShaders;
    
    // rendering
    bool mDrawOnSphere;
    ci::gl::Fbo mShaderFbo;
    
    // shader params
    struct tMetaHexParams
    {
        float mSpeed;
        float mLightSpeed;
        float mObjTime;
        float mLightTime;
        int mNumObjects;
        int mRenderSteps;
        int mQuality;
        ci::Vec3f mCoeffecients;
        bool mAudioCoeffs;
    };
    tMetaHexParams mMetaHexParams;
    
    struct tRetinaParams
    {
        bool                mAudioDialation;
        float               mScale;
        float               mDialation;
        float               mDialationScale;
        float               mPatternAmp;
        float               mPatternFreq;
        bool                mAudioPattern;
    };
    tRetinaParams mRetinaParams;
    
    struct tFireballParams
    {
        float               mRotationSpeed;
        float               mDensity;
        bool                mAudioDensity;
        bool                mAudioRotation;
    };
    tFireballParams mFireballParams;
    
    struct tBioFractalParams
    {
        int         mIterations;
        ci::Anim<ci::Vec3f>  mJulia;
        ci::Anim<ci::Vec3f>  mRotation;
        ci::Vec3f   mLightDir;
        float       mScale;
        float       mRotAngle;
        float       mAmplitude;
        float       mDetail;
        float       mAnimTime;
        bool        mCycleJulia;
        bool        mCycleRotation;
        
    };
    tBioFractalParams mBioFractalParams;
};

