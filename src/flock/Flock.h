//
//  Flock.h
//  Oculon
//
//  Created by Ehsan Rezaie on 12-07-21.
//
//

#pragma once

#include "Scene.h"
#include "SpringCam.h"
#include "SplineCam.h"
#include "Controller.h"

#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"

class Flock : public Scene
{
public:
	Flock();
    virtual ~Flock();
    
    // inherited from Scene
    void setup();
    void reset();
    void resize();
    void update(double dt);
    void draw();
    void drawDebug();
    void handleMouseDown( const ci::app::MouseEvent& event );
    void handleMouseUp( const ci::app::MouseEvent& event );
    void handleMouseDrag( const ci::app::MouseEvent& event );
    void handleMouseMove( const ci::app::MouseEvent& event );
    const ci::Camera& getCamera();
    
protected:
    ////////------------------------------------------------------
    //
	void				adjustFboDim( int offset );
	void				initialize();
	void				setFboPositions( ci::gl::Fbo fbo );
	void				setFboVelocities( ci::gl::Fbo fbo );
	void				setPredatorFboPositions( ci::gl::Fbo fbo );
	void				setPredatorFboVelocities( ci::gl::Fbo fbo );
	void				initVbo();
	void				initPredatorVbo();
	void				drawIntoRoomFbo();
	void				drawInfoPanel();
	void				drawIntoVelocityFbo(double dt);
	void				drawIntoPositionFbo(double dt);
	void				drawIntoPredatorVelocityFbo(double dt);
	void				drawIntoPredatorPositionFbo(double dt);
	void				drawIntoLanternsFbo();
	void				drawGlows( const ci::Vec3f& billboardRight, const ci::Vec3f& billboardUp );
	void				drawNebulas( const ci::Vec3f& billboardRight, const ci::Vec3f& billboardUp );
    //
    ////////------------------------------------------------------
    
    void setupInterface();
    void setupDebugInterface();
	
private:
    ////////------------------------------------------------------
    //
	// CAMERA
	SpringCam			mSpringCam;
    SplineCam           mSplineCam;
	
	// TEXTURES
	ci::gl::Texture			mLanternGlowTex;
	ci::gl::Texture			mGlowTex;
	ci::gl::Texture			mNebulaTex;
	ci::gl::Texture			mIconTex;
	
	// SHADERS
	ci::gl::GlslProg		mVelocityShader;
	ci::gl::GlslProg		mPositionShader;
	ci::gl::GlslProg		mP_VelocityShader;
	ci::gl::GlslProg		mP_PositionShader;
	ci::gl::GlslProg		mLanternShader;
	ci::gl::GlslProg		mRoomShader;
	ci::gl::GlslProg		mShader;
	ci::gl::GlslProg		mP_Shader;
	ci::gl::GlslProg		mGlowShader;
	ci::gl::GlslProg		mNebulaShader;
	
	// CONTROLLER
	Controller              mController;
    
	// LANTERNS (point lights)
	ci::gl::Fbo				mLanternsFbo;
	
	// VBOS
	ci::gl::VboMesh			mVboMesh;
	ci::gl::VboMesh			mP_VboMesh;
	
	// POSITION/VELOCITY FBOS
	ci::gl::Fbo::Format		mRgba16Format;
	int					mFboDim;
	ci::Vec2f			mFboSize;
	ci::Area			mFboBounds;
	ci::gl::Fbo			mPositionFbos[2];
	ci::gl::Fbo			mVelocityFbos[2];
	int					mP_FboDim;
	ci::Vec2f			mP_FboSize;
	ci::Area			mP_FboBounds;
	ci::gl::Fbo			mP_PositionFbos[2];
	ci::gl::Fbo			mP_VelocityFbos[2];
	int					mThisFbo, mPrevFbo;
    
    // MOUSE
    ci::Vec2f			mMousePos, mMouseDownPos, mMouseOffset;
	bool				mMousePressed;
    
    bool                mInitUpdateCalled;
    bool                mDrawNebulas;
    //
    ////////------------------------------------------------------
    
#define FLOCK_CAMTYPE_TUPLE \
FLOCK_CAMTYPE_ENTRY( "Manual", CAM_MANUAL ) \
FLOCK_CAMTYPE_ENTRY( "Orbiter", CAM_ORBITER ) \
FLOCK_CAMTYPE_ENTRY( "Graviton", CAM_GRAVITON ) \
FLOCK_CAMTYPE_ENTRY( "Catalog", CAM_CATALOG ) \
FLOCK_CAMTYPE_ENTRY( "Spring", CAM_SPRING ) \
FLOCK_CAMTYPE_ENTRY( "Spline", CAM_SPLINE ) \
//end tuple
    
    enum eCamType
    {
#define FLOCK_CAMTYPE_ENTRY( nam, enm ) \
enm,
        FLOCK_CAMTYPE_TUPLE
#undef  FLOCK_CAMTYPE_ENTRY
        
        CAM_COUNT
    };
    eCamType                    mCamType;
    
    bool mDrawPredators;
    
    float mTimeScale;
    
};

