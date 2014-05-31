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
#include "Scene.h"
#include "MotionBlurRenderer.h"
#include "GridRenderer.h"
#include "AudioInputHandler.h"
#include "FragShader.h"
#include "TimeController.h"

//
// Object Shaders
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
    void shaderPostDraw();
    
private:
    
    TimeController      mTimeController;
    
    // global params
    ci::ColorAf         mColor1;
    ci::ColorAf         mColor2;
    ci::ColorAf         mColor3;
    
    // color maps
    typedef std::pair<std::string, ci::gl::Texture> tNamedTexture;
    std::vector<tNamedTexture> mColorMaps;
    int mColorMapIndex;
    
    // noise texture
    ci::gl::Texture     mNoiseTexture;
    
    std::vector<FragShader*> mShaders;
    int mShaderType;
    
    // rendering
    //    bool mDrawOnSphere;
    //    ci::gl::Fbo mShaderFbo;
};

#pragma mark - Hexballs

class MetaballsShader : public FragShader
{
public:
    MetaballsShader();
    
    virtual void setupInterface( Interface* interface, const std::string& name );
    virtual void update(double dt);
    virtual void setCustomParams( AudioInputHandler& audioInputHandler );
    
private:
    float mSpeed;
    float mLightSpeed;
    float mObjTime;
    float mLightTime;
    int mNumObjects;
    int mRenderSteps;
    int mQuality;
    ci::Vec3f mCoeffecients;
    int mResponseBand;
};

#pragma mark - Lissajous

class LissajousShader : public FragShader
{
public:
    LissajousShader();
    
    virtual void setupInterface( Interface* interface, const std::string& name );
    virtual void setCustomParams( AudioInputHandler& audioInputHandler );
    
private:
    float mScale;
    int mFrequencyX;
    int mFrequencyY;
    
    float mFrequencyXShift;
    float mFrequencyYShift;
    
    int mResponseBandX;
    int mResponseBandY;
    
    ci::ColorAf mColor;
};


#pragma mark - Retina

class RetinaShader : public FragShader
{
public:
    RetinaShader();
    
    virtual void setupInterface( Interface* interface, const std::string& name );
    virtual void setCustomParams( AudioInputHandler& audioInputHandler );
    
private:
    float               mScale;
    float               mDialation;
    float               mDialationScale;
    float               mPatternAmp;
    float               mPatternFreq;
    int                 mDialationBand;
    int                 mPatternBand;
};

#pragma mark - BioFractal

class BioFractalShader : public FragShader
{
public:
    BioFractalShader();
    
    virtual void setupInterface( Interface* interface, const std::string& name );
    virtual void setCustomParams( AudioInputHandler& audioInputHandler );
    
private:
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
    
    int         mRotXBand;
    int         mRotYBand;
    int         mRotZBand;
    int         mAmplitudeBand;
};

#pragma mark - Gyroid

class GyroidShader : public FragShader
{
public:
    GyroidShader();
    
    virtual void setupInterface( Interface* interface, const std::string& name );
    virtual void setCustomParams( AudioInputHandler& audioInputHandler );
    
private:
    int                 mMaxSteps;
    float               mFieldOfView;
    float               mScale;
    int                 mScaleResponseBand;
    float               mAmbientLight;
    float               mDiffuseLight;
    float               mSpecularLight;
    ci::Vec3f           mLight1Dir;
    ci::Vec3f           mLight2Dir;
    bool                mInverted;
};

#pragma mark - Terminal

class TerminalShader : public FragShader
{
public:
    TerminalShader();
    
    virtual void setupInterface( Interface* interface, const std::string& name );
    virtual void setCustomParams( AudioInputHandler& audioInputHandler );
    
private:
    
};

