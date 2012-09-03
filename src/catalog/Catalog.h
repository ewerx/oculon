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

using namespace ci;

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
    
    void drawHud();
    
    ////////------------------------------------------------------
    //
    void        initBrightVbo();
	void        initFaintVbo();
    void        setFboPositions( gl::Fbo &fbo );
	void        parseStarData( const fs::path &path );
	Vec3f       convertToCartesian( double ra, double dec, double dist );
	void        createStar( const std::string &text, int lineNumber );
	void        setView( int homeIndex, int destIndex );
    void        selectStar( bool wasRightClick );
    //
    ////////------------------------------------------------------
    void        parsePlanetData( const fs::path &path );
	bool        createPlanet( const std::string &text, int lineNumber );
	
    void        setHomeStar( Star* target );
    void        setDestStar( Star* target );
    bool        setRandomHome();
    bool        setRandomDest();
        
private:
    
    ////////------------------------------------------------------
    //
    // CAMERA
	SpringCam			mSpringCam;
    StarCam             mStarCam;
	
	// SHADERS
	gl::GlslProg		mBrightStarsShader;
	gl::GlslProg		mFaintStarsShader;
	
	// TEXTURES
	gl::Texture			mIconTex;
	gl::Texture			mMilkyWayTex;
	gl::Texture			mSpectrumTex;
	
	// POSITION/VELOCITY FBOS
	gl::Fbo				mPositionFbo;
	
	// FONT
	Font				mFontBlackT, mFontBlackS;
	gl::TextureFontRef	mTextureFontT, mTextureFontS;
	
	// MOUSE
	Vec2f				mMousePos, mMouseDownPos, mMouseOffset;
	bool				mMousePressed, mMouseRightPressed;
	float				mMouseTimePressed;
	float				mMouseTimeReleased;
	float				mMouseTimeThresh;
	bool				mWasRightButtonLastClicked;
    
	// STARS
	gl::VboMesh			mBrightVbo;
	gl::VboMesh			mFaintVbo;
	
	std::vector<Star*>	mStars;
	std::vector<Star*>	mBrightStars;
	std::vector<Star*>	mFaintStars;
	std::vector<Star*>	mNamedStars;
	std::vector<Star*>	mTouringStars;
    typedef std::map<unsigned long,Star*> tStarMap;
    tStarMap mStarsHIP;
    tStarMap mStarsHD;
    std::vector<Star*>  mStarsWithPlanets;
	
	gl::Texture			mStarTex;
	gl::Texture			mStarGlowTex;
	gl::Texture			mDarkStarTex;
	
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
    
    enum eCamType
    {
        CAM_SPRING,
        CAM_STAR,
        CAM_ORBITER,
        
        CAM_COUNT
    };
    eCamType mCamType;
    float mCameraDistance;
    
    // display list
    //GLuint mDisplayListPoints;
        
    // hud
    /*
    enum eTextBoxLocations
    {
        TB_TOP_LEFT,
        TB_TOP_RIGHT,
        TB_BOT_LEFT,
        TB_BOT_RIGHT,
        
        TB_COUNT
    };
    TextEntity*     mTextBox[TB_COUNT];
    */
    
    static const int FBO_WIDTH;
    static const int FBO_HEIGHT;
    
};
