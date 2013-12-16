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
    bool mShowBackground;
    bool mShowLabels;
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
    StarCam             mStarCam;
    SplineCam           mSplineCam;
    
    float               mStarCamRotateSpeed;
    float               mStarCamTimeScale;
    
    // camera
#define PARSEC_CAMTYPE_TUPLE \
PARSEC_CAMTYPE_ENTRY( "Manual", CAM_MANUAL ) \
PARSEC_CAMTYPE_ENTRY( "Orbiter", CAM_ORBITER ) \
PARSEC_CAMTYPE_ENTRY( "Star", CAM_STAR ) \
PARSEC_CAMTYPE_ENTRY( "Spline", CAM_SPLINE ) \
PARSEC_CAMTYPE_ENTRY( "Catalog", CAM_CATALOG ) \
//end tuple
    
    enum eCamType
    {
#define PARSEC_CAMTYPE_ENTRY( nam, enm ) \
enm,
        PARSEC_CAMTYPE_TUPLE
#undef  PARSEC_CAMTYPE_ENTRY
        
        CAM_COUNT
    };
    eCamType                    mCamType;
    
    // audio
    AudioInputHandler   mAudioInputHandler;

};