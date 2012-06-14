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

using namespace ci;

//
// Audio input tests
//
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
    
    // callbacks
    
protected:
    void setupInterface();
    void setupDebugInterface();
    
    void initStars();
    void drawHud();
    
    ////////------------------------------------------------------
    //
    void        initBrightVbo();
	void        initFaintVbo();
    void        setFboPositions( gl::Fbo &fbo );
	void        parseData( const fs::path &path );
	Vec3f       convertToCartesian( double ra, double dec, double dist );
	void        createStar( const std::string &text, int lineNumber );
	void        setView( int homeIndex, int destIndex );
    void        selectStar( bool wasRightClick );
    //
    ////////------------------------------------------------------
        
private:
    
    ////////------------------------------------------------------
    //
    // CAMERA
	SpringCam			mSpringCam;
	
	// SHADERS
	gl::GlslProg		mRoomShader;
	gl::GlslProg		mBrightStarsShader;
	gl::GlslProg		mFaintStarsShader;
	
	// TEXTURES
	gl::Texture			mIconTex;
	gl::Texture			mMilkyWayTex;
	gl::Texture			mSpectrumTex;
	
	// ROOM
	//Room				mRoom;
	gl::Fbo				mRoomFbo;
	
	// ROOM PANEL
	gl::Texture			mRoomNumTex;
	gl::Texture			mRoomDescTex;
	
	// POSITION/VELOCITY FBOS
	gl::Fbo				mPositionFbo;
	
	// FONT
	Font				mFontBlackT, mFontBlackS, mFontBlackM, mFontBlackL;
	gl::TextureFontRef	mTextureFontT, mTextureFontS, mTextureFontM, mTextureFontL;
	
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
	
	std::vector<Star>	mStars;
	std::vector<Star*>	mBrightStars;
	std::vector<Star*>	mFaintStars;
	std::vector<Star*>	mNamedStars;
	std::vector<Star*>	mTouringStars;
	
	gl::Texture			mStarTex;
	gl::Texture			mStarGlowTex;
	gl::Texture			mDarkStarTex;
	
	float				mScale, mScaleDest;
	float				mScalePer, mMaxScale;
	
	int					mTotalTouringStars;
	int					mTouringStarIndex;
	Star				*mHomeStar;
	Star				*mDestStar;
    
	float				mDataTimer;
	
	// RENDER OPTIONS
	bool				mRenderNames;
	bool				mRenderFaintStars;
	bool				mRenderBrightStars;
    //
    ////////------------------------------------------------------
    
    // rendering
    bool mShowLabels;
    
    // display list
    //GLuint mDisplayListPoints;
        
    // hud
    enum eTextBoxLocations
    {
        TB_TOP_LEFT,
        TB_TOP_RIGHT,
        TB_BOT_LEFT,
        TB_BOT_RIGHT,
        
        TB_COUNT
    };
    TextEntity*     mTextBox[TB_COUNT];
    
    static const int FBO_WIDTH;
    static const int FBO_HEIGHT;
    
};
