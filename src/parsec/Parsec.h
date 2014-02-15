//
//  Parsec.h
//  Oculon
//
//  Created by Ehsan Rezaie on 12/7/2013.
//
//

#pragma once

#include <vector>
#include "Scene.h"
#include "Constellations.h"
#include "ConstellationArt.h"
#include "ParsecBackground.h"
#include "ParsecGrid.h"
#include "ParsecStars.h"
#include "ParsecLabels.h"
#include "StarCam.h"
#include "SplineCam.h"
#include "AudioInputHandler.h"
#include "CameraController.h"

class Parsec : public Scene
{
public:
    Parsec();
    virtual ~Parsec();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    const ci::Camera& getCamera();
    
protected:// from Scene
    void setupInterface();
    //void setupDebugInterface();
    
protected:
    void loadData();
    void render();
    
protected:
    // params
    bool mShowGrid;
    bool mShowConstellations;
    bool mShowConstellationArt;
    float mConstellationAlpha;
    bool mConstellationAudio;
    
    // guts
    ParsecStars         mStars;
    ParsecLabels        mLabels;
    ParsecBackground    mBackground;
    ParsecGrid          mGrid;
    Constellations      mConstellations;
    ConstellationArt    mConstellationArt;
  
    // frame buffer and shader used for cylindrical projection
    ci::gl::Fbo			mProjectionFbo;
    ci::gl::GlslProg	mProjectionShader;
	unsigned			mSectionCount;
	float				mSectionFovDegrees;
	float				mSectionOverlap;
    
    // cameras
    CameraController    mCameraController;
    
    // audio
    AudioInputHandler   mAudioInputHandler;

};