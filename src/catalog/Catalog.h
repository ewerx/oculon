/*
 *  Catalog.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#pragma once

#include "Scene.h"
#include "SpringCam.h"
#include "StarCam.h"
#include "Star.h"
#include "TextEntity.h"

#include "cinder/Cinder.h"
#include "cinder/audio/Input.h"
#include "cinder/Timeline.h"
#include "cinder/Text.h"
#include "cinder/Font.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/Camera.h"
#include "cinder/Rand.h"

#include <vector>
#include <map>

class Catalog : public Scene
{
public:
    Catalog();
    virtual ~Catalog();
    
    // inherited from Scene
    void setup();
    void reset();
    void resize();
    void update(double dt);
    void draw();
    void drawDebug();
    bool handleKeyDown(const ci::app::KeyEvent& keyEvent);
    void handleMouseDown( const ci::app::MouseEvent& event );
    void handleMouseUp( const ci::app::MouseEvent& event );
    void handleMouseDrag( const ci::app::MouseEvent& event );
    void handleMouseMove( const ci::app::MouseEvent& event );
    const ci::Camera& getCamera();
    
    // callbacks
    bool setCamType();
    
protected:
    void setupInterface();
    void setupDebugInterface();
    
    ////////------------------------------------------------------
    //
    void        initBrightVbo();
	void        initFaintVbo();
    void        setFboPositions( ci::gl::Fbo &fbo );
	void        parseStarData( const ci::fs::path &path );
    ci::Vec3f   convertToCartesian( double ra, double dec, double dist );
	void        createStar( const std::string &text, int lineNumber );
	void        setView( int homeIndex, int destIndex );
    void        selectStar( bool wasRightClick );
    //
    ////////------------------------------------------------------
    void        parsePlanetData( const ci::fs::path &path );
	bool        createPlanet( const std::string &text, int lineNumber );
	
    void        setHomeStar( Star* target );
    void        setDestStar( Star* target );
    bool        setRandomHome();
    bool        setRandomDest();
    bool        setSolHome();
    bool        setSolDest();
    bool        setNextKeplerStar();
    bool        setPrevKeplerStar();
    bool        setKeplerStar();
    
    void        updateAudioResponse();
        
private:
    
    ////////------------------------------------------------------
    //
    // CAMERA
	SpringCam			mSpringCam;
    StarCam             mStarCam;
	
	// SHADERS
	ci::gl::GlslProg		mBrightStarsShader;
	ci::gl::GlslProg		mFaintStarsShader;
	
	// TEXTURES
	ci::gl::Texture			mIconTex;
	ci::gl::Texture			mMilkyWayTex;
	ci::gl::Texture			mSpectrumTex;
	
	// POSITION/VELOCITY FBOS
	ci::gl::Fbo				mPositionFbo;
	
	// FONT
	ci::Font				mFontBlackT, mFontBlackS;
	ci::gl::TextureFontRef	mTextureFontT, mTextureFontS;
	
	// MOUSE
	ci::Vec2f				mMousePos, mMouseDownPos, mMouseOffset;
	bool				mMousePressed, mMouseRightPressed;
	float				mMouseTimePressed;
	float				mMouseTimeReleased;
	float				mMouseTimeThresh;
	bool				mWasRightButtonLastClicked;
    
	// STARS
	ci::gl::VboMesh			mBrightVbo;
	ci::gl::VboMesh			mFaintVbo;
	
	std::vector<Star*>	mStars;
	std::vector<Star*>	mBrightStars;
	std::vector<Star*>	mFaintStars;
	std::vector<Star*>	mNamedStars;
	std::vector<Star*>	mTouringStars;
    typedef std::map<unsigned long,Star*> tStarMap;
    tStarMap mStarsHIP;
    tStarMap mStarsHD;
    std::vector<Star*>  mStarsWithPlanets;
	
	ci::gl::Texture			mStarTex;
	ci::gl::Texture			mStarGlowTex;
	ci::gl::Texture			mDarkStarTex;
	
	float				mScale, mScaleDest;
	float				mScalePer, mMaxScale;
	
	int					mTotalTouringStars;
	int					mTouringStarIndex;
	Star				*mHomeStar;
	Star				*mDestStar;
    Star                *mSol;
    
	float				mDataTimer;
	
	// RENDER OPTIONS
	bool				mRenderNames;
	bool				mRenderFaintStars;
	bool				mRenderBrightStars;
    //
    ////////------------------------------------------------------
    
    // rendering
    bool mMoreGlow;
    bool mShowSol;
    
#define CATALOG_CAMTYPE_TUPLE \
CATALOG_CAMTYPE_ENTRY( "Manual", CAM_MANUAL ) \
CATALOG_CAMTYPE_ENTRY( "Spring", CAM_SPRING ) \
CATALOG_CAMTYPE_ENTRY( "Orbiter", CAM_ORBITER ) \
CATALOG_CAMTYPE_ENTRY( "Star", CAM_STAR ) \
//end tuple
    
    enum eCamType
    {
#define CATALOG_CAMTYPE_ENTRY( nam, enm ) \
enm,
        CATALOG_CAMTYPE_TUPLE
#undef  CATALOG_CAMTYPE_ENTRY
        
        CAM_COUNT
    };
    eCamType mCamType;
    
    float mCameraDistance;
    
    float mLabelBrightnessByAudio;
    
    int mNextKeplerIndex;
    float mStarCamTimeScale;
    
    float mStarfieldAlpha;
    float mNamesAlpha;
    
    // display list
    //GLuint mDisplayListPoints;
    
    static const int FBO_WIDTH;
    static const int FBO_HEIGHT;
    
};
