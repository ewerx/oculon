//
// Orbiter.h
// Oculon
//
// Created by Ehsan on 11-10-19.
// Copyright 2011 ewerx. All rights reserved.
//

#pragma once

#include "Scene.h"
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/gl/TextureFont.h"
#include "CameraController.h"
#include "TimeController.h"
#include <vector>

class Body;
class Sun;
class TextEntity;
class Star;

//
// An orbital simulator
//
class Orbiter : public Scene
{
public:
    Orbiter();
    virtual ~Orbiter();
    
    // inherited from Scene
    void setup();
    void reset();
    void resize();
    void update(double dt);
    void draw();
    bool handleKeyDown(const ci::app::KeyEvent& keyEvent);
    const ci::Camera& getCamera();
    
    // callbacks
    bool prevTarget();
    bool nextTarget();
    
    void createSystem( Star* star );
    
    
    // HACK
    enum eCamType
    {
        CAM_MANUAL,
        CAM_FOLLOW,
        CAM_CATALOG,
        CAM_BINNED,
        
        CAM_COUNT
    };
    eCamType getCamType() { return mCamType; }
    
    bool isDrawLabelsEnabled()              { return mDrawLabels; }
    ci::gl::TextureFontRef getLabelFont()   { return mTextureFontLabel; }
    ci::gl::TextureFontRef getHudFont()     { return mTextureFontHud; }
   
    Star* getExoStar() { return mExoStar; }
    
    float getMinRadiusMultiplier() const        { return mMinRadiusMultiplier; }
    float getMaxRadiusMultiplier() const        { return mMaxRadiusMultiplier; }
    float getFalloff() const                    { return mFalloff; }
    float getLabelBrightnessByAudio() const     { return mLabelBrightnessByAudio; }
protected:// from Scene
    void setupInterface();
    //void setupDebugInterface();
    
private:
    
    void updateAudioResponse();
    void updateCam();
    
    void setupHud();
    void updateHud();
    void drawHud();
    void drawHudSpectrumAnalyzer(float left, float top, float width, float height);
    
    void removeBodies();
    void createSolSystem();
    
private:
    //Vec3f           mLightDirection;
    
    typedef std::vector<Body*> BodyList;
    BodyList            mBodies;
    
    enum { NUM_PLANETS = 8 };
    ci::ImageSourceRef      mTextures[NUM_PLANETS];
    
    Sun*                mSun;
    int                 mFollowTargetIndex;
    Body*               mFollowTarget;
    ci::CameraPersp         mCam;
    ci::Matrix44d           mScaleMatrix;
    
    int             mFrequencySpread;
    float           mMinRadiusMultiplier;
    float           mMaxRadiusMultiplier;
    float           mFalloff;
    
    eCamType        mCamType;
    
    double          mElapsedTime;
    double          mTimeScale;
    double          mDrawScale;
    double          mGravityConstant;
    
    ci::Quatd           mCameraRotation;
    
    ci::Font                mFontLabel;
    ci::Font                mFontHud;
    ci::gl::TextureFontRef  mTextureFontLabel;
    ci::gl::TextureFontRef  mTextureFontHud;
    
    enum eTextBoxLocations
    {
        TB_TOP_LEFT,
        TB_TOP_RIGHT,
        TB_BOT_LEFT,
        TB_BOT_RIGHT,
        
        TB_COUNT
    };
    TextEntity*     mTextBox[TB_COUNT];
    bool            mDrawHud;
    bool            mDrawLabels;
    bool            mDrawTrails;
    
    float           mLabelBrightnessByAudio;
    
    Star*           mExoStar;
    
    // NEW
    
    AudioInputHandler   mAudioInputHandler;
    
    // camera
    CameraController    mCameraController;
    
    TimeController      mTimeController;
    
public:
    static double   sDefaultTimeScale;
    static double   sDefaultGravityConstant;
    static double   sDrawScale;
    static bool     sUseSmoothLines;
    static bool     sUseTriStripLine;
    static int      sMinTrailLength;
    static float    sTrailWidth;
    static bool     sDrawRealSun;
    static float    sPlanetGrayScale;
};

