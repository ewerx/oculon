/*
 *  Corona.h
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
#include "CoronaStar.h"
#include "CoronaController.h"
#include "CubeMap.h"

#include "CameraController.h"
#include "TimeController.h"

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

//
// Audio input tests
//
class Corona : public Scene
{
public:
    Corona();
    virtual ~Corona();
    
    // inherited from Scene
    void setup();
    void reset();
    void resize();
    void update(double dt);
    void draw();
    //void drawDebug();
    bool handleKeyDown(const ci::app::KeyEvent& keyEvent);
    void handleMouseDown( const ci::app::MouseEvent& event );
    void handleMouseUp( const ci::app::MouseEvent& event );
    void handleMouseDrag( const ci::app::MouseEvent& event );
    void handleMouseMove( const ci::app::MouseEvent& event );
    const ci::Camera& getCamera();
    
    ///////------------------------------------------------------
    //
    void			createSphere( gl::VboMesh &mesh, int res );
	void			drawSphereTri( ci::Vec3f va, ci::Vec3f vb, ci::Vec3f vc, int div );
    void			toggleCanisMajoris();
	void			randSeed();
	void			setStage( int i );
    void			drawCorona();
	void			drawFlat();
	void			drawSphere();
	void			drawGlows();
	void			drawNebulas();
	void			drawDusts();
	void			drawInfoPanel();
	void			drawCanisMajoris();
	void			setStar( int i );
    //
    ////////------------------------------------------------------
    
protected:
    void setupInterface();
    //void setupDebugInterface();
        
private:
    ///////------------------------------------------------------
    //
    enum stageName
    { INTRO, CIRCLE, GRADIENT, CORONA, SPHERE, NEBULAS, GLOWS, DUSTS, EVERYTHING, LESSON, STAGE_COUNT };
	
	// SHADERS
	gl::GlslProg		mGradientShader;
	gl::GlslProg		mStarShader;
	gl::GlslProg		mGlowShader;
	gl::GlslProg		mNebulaShader;
	gl::GlslProg		mDustShader;
	gl::GlslProg		mCoronaShader;
	gl::GlslProg		mPlanetShader;
	
	// TEXTURES
    CubeMap             mCubeMap;
	gl::Texture			mSpectrumTex;
	gl::Texture			mGlowTex;
	gl::Texture			mNebulaTex;
	gl::Texture			mCoronaTex;
	gl::Texture			mGridTex;
	gl::Texture			mSmallGridTex;
	gl::Texture			mBigGlow0Tex;
	gl::Texture			mBigGlow1Tex;
	gl::Texture			mIconTex;
	
	// STAR
	CoronaStar          mStar;
	gl::VboMesh			mStarVbo;
	gl::VboMesh			mStarVboLo;
	std::vector<Vec3f>	mPosCoords;
	std::vector<Vec3f>	mNormals;
	Vec3f				mCanisMajorisPos;
	float				mCanisMajorisPer;
    
	// CONTROLLER
	CoronaController	mController;
	
	// POSITION/VELOCITY FBOS
	ci::Vec2f			mFboSize;
	ci::Area			mFboBounds;
	int					mThisFbo, mPrevFbo;
	
	// FONT
	Font				mFontBlackS, mFontBlackM, mFontBlackL;
	gl::TextureFontRef	mTextureFontS, mTextureFontM, mTextureFontL;
	
	int					mStage;
    std::vector<std::string>		mTitles;
	
	int					mNumStarTypes;
	int					mStarTypeIndex;
    std::vector<std::string>		mStarNames;
	std::vector<int>			mStarTemps;
	std::vector<float>		mStarRadii;
	std::vector<float>		mStarColors;
	std::vector<float>		mStarDists;
	std::vector<int>			mStarPlanets;
	
	int					mRandIterations;
    
	Vec2f				mRandSeed;
	bool				mBillboard;
	
	bool				mRenderGlows;
	bool				mRenderNebulas;
	bool				mRenderSphere;
	bool				mRenderTexture;
	bool				mRenderDusts;
	bool				mRenderCorona;
	bool				mRenderCanisMajoris;
	
	// MOUSE
	Vec2f				mMousePos, mMouseDownPos, mMouseOffset;
	bool				mMousePressed;
    //
    ////////------------------------------------------------------
    
    TimeController      mTimeController;
    
    // camera
    CameraController mCameraController;
};
