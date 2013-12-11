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
#include "ParsecBackground.h"
#include "ParsecGrid.h"
#include "ParsecStars.h"
#include "StarCam.h"
#include "SplineCam.h"

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
    
    // guts
    ParsecStars         mStars;
    ParsecBackground    mBackground;
    ParsecGrid          mGrid;
  
    // frame buffer and shader used for cylindrical projection
    ci::gl::Fbo			mProjectionFbo;
    ci::gl::GlslProg	mProjectionShader;
	unsigned			mSectionCount;
	float				mSectionFovDegrees;
	float				mSectionOverlap;
    
    // cameras
//    ci::CameraPersp     mStarCam;
    StarCam             mStarCam;
    SplineCam           mSplineCam;
    
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

};