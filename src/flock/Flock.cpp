//
//  Flock.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 12-07-04.
//  Copyright (c) 2012 ewerx. All rights reserved.
//

// this code adapted from Robert Hodgin's Eyeo Flocking demo

#include "Flock.h"
#include "OculonApp.h"
#include "AudioInput.h"
#include "Interface.h"
#include "Resources.h"
#include "SpringCam.h"
#include "Controller.h"
#include "Lantern.h"

#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"
#include "cinder/Camera.h"
#include "cinder/Rand.h"


using namespace ci;
using namespace ci::app;
using namespace std;

#define FBO_DIM			42//167
#define P_FBO_DIM		5
#define MAX_LANTERNS	5

// ----------------------------------------------------------------
//
Flock::Flock()
: Scene("flock")
{
}

// ----------------------------------------------------------------
//
Flock::~Flock()
{
}

void Flock::setup()
{
    mCamType = CAM_SPRING;
    mDrawNebulas = false;
    mDrawPredators = false;
    mTimeScale = 1.0f;
    
    ////////------------------------------------------------------
    //
    // CAMERA
	mSpringCam			= SpringCam( -420.0f, mApp->getViewportAspectRatio(), 3000.0f );
    mSplineCam.setup(8000.0f,1000.0f);
    
	// POSITION/VELOCITY FBOS
	mRgba16Format.setColorInternalFormat( GL_RGBA16F_ARB );
	mRgba16Format.setMinFilter( GL_NEAREST );
	mRgba16Format.setMagFilter( GL_NEAREST );
	mThisFbo			= 0;
	mPrevFbo			= 1;
	
	// LANTERNS
	mLanternsFbo		= gl::Fbo( MAX_LANTERNS, 2, mRgba16Format );
	
	// TEXTURE FORMAT
	gl::Texture::Format mipFmt;
    mipFmt.enableMipmapping( true );
    mipFmt.setMinFilter( GL_LINEAR_MIPMAP_LINEAR );    
    mipFmt.setMagFilter( GL_LINEAR );
	
	// TEXTURES
	mLanternGlowTex		= gl::Texture( loadImage( loadResource( RES_FLOCK_LANTERNGLOW_PNG ) ) );
	mGlowTex			= gl::Texture( loadImage( loadResource( RES_FLOCK_GLOW_PNG ) ) );
	mNebulaTex			= gl::Texture( loadImage( loadResource( RES_FLOCK_NEBULA_PNG ) ) );
	
	// LOAD SHADERS
	try {
		mVelocityShader		= gl::GlslProg( loadResource( RES_FLOCK_PASSTHRU_VERT ),	loadResource( RES_FLOCK_VELOCITY_FRAG ) );
		mPositionShader		= gl::GlslProg( loadResource( RES_FLOCK_PASSTHRU_VERT ),	loadResource( RES_FLOCK_POSITION_FRAG ) );
		mP_VelocityShader	= gl::GlslProg( loadResource( RES_FLOCK_PASSTHRU_VERT ),	loadResource( RES_FLOCK_P_VELOCITY_FRAG ) );
		mP_PositionShader	= gl::GlslProg( loadResource( RES_FLOCK_PASSTHRU_VERT ),	loadResource( RES_FLOCK_P_POSITION_FRAG ) );
		mLanternShader		= gl::GlslProg( loadResource( RES_FLOCK_LANTERN_VERT ),	loadResource( RES_FLOCK_LANTERN_FRAG ) );
		//mRoomShader			= gl::GlslProg( loadResource( RES_FLOCK_ROOM_VERT ),loadResource( RES_FLOCK_ROOM_FRAG ) );
		mShader				= gl::GlslProg( loadResource( RES_FLOCK_VBOPOS_VERT ),	loadResource( RES_FLOCK_VBOPOS_FRAG ) );
		mP_Shader			= gl::GlslProg( loadResource( RES_FLOCK_P_VBOPOS_VERT ),	loadResource( RES_FLOCK_P_VBOPOS_FRAG ) );
		mGlowShader			= gl::GlslProg( loadResource( RES_FLOCK_PASSTHRU_VERT ),	loadResource( RES_FLOCK_GLOW_FRAG ) );
		mNebulaShader		= gl::GlslProg( loadResource( RES_FLOCK_PASSTHRU_VERT ),	loadResource( RES_FLOCK_NEBULA_FRAG ) );
	} catch( gl::GlslProgCompileExc e ) {
		std::cout << e.what() << std::endl;
		exit(1);
	}
	
	// CONTROLLER
	mController			= Controller( MAX_LANTERNS );
    
    // MOUSE
	mMousePos			= Vec2f::zero();
	mMouseDownPos		= Vec2f::zero();
	mMouseOffset		= Vec2f::zero();
	mMousePressed		= false;

	mInitUpdateCalled	= false;
	
	initialize();

    //
    ///////--------------------------------------------------------
    Scene::setup();

    reset();
}

// ----------------------------------------------------------------
//
void Flock::setupInterface()
{
    mInterface->gui()->addColumn();
    vector<string> camTypeNames;
#define FLOCK_CAMTYPE_ENTRY( nam, enm ) \
camTypeNames.push_back(nam);
    FLOCK_CAMTYPE_TUPLE
#undef  FLOCK_CAMTYPE_ENTRY
    mInterface->addEnum(CreateEnumParam( "Cam Type", (int*)(&mCamType) )
                        .maxValue(CAM_COUNT)
                        .oscReceiver(getName(), "camtype")
                        .isVertical(), camTypeNames);
    
    mInterface->addParam(CreateBoolParam("draw nebulas", &mDrawNebulas));
    mInterface->addParam(CreateBoolParam("draw predators", &mDrawPredators));
    mInterface->addParam(CreateFloatParam("time scale", &mTimeScale, 0.001f, 10.0f));
    
    mController.setupInterface(mInterface);
    mSplineCam.setupInterface(mInterface, mName);
}

// ----------------------------------------------------------------
//
void Flock::setupDebugInterface()
{
}

// ----------------------------------------------------------------
//
void Flock::reset()
{
    
}

// ----------------------------------------------------------------
//
void Flock::resize()
{
    Scene::resize();
}

////////------------------------------------------------------
//
void Flock::initialize()
{
	gl::disableAlphaBlending();
	gl::disableDepthWrite();
	gl::disableDepthRead();
	
	mFboDim				= FBO_DIM;
	mFboSize			= Vec2f( mFboDim, mFboDim );
	mFboBounds			= Area( 0, 0, mFboDim, mFboDim );
	mPositionFbos[0]	= gl::Fbo( mFboDim, mFboDim, mRgba16Format );
	mPositionFbos[1]	= gl::Fbo( mFboDim, mFboDim, mRgba16Format );
	mVelocityFbos[0]	= gl::Fbo( mFboDim, mFboDim, mRgba16Format );
	mVelocityFbos[1]	= gl::Fbo( mFboDim, mFboDim, mRgba16Format );
	
	setFboPositions( mPositionFbos[0] );
	setFboPositions( mPositionFbos[1] );
	setFboVelocities( mVelocityFbos[0] );
	setFboVelocities( mVelocityFbos[1] );
	
	
	mP_FboDim			= P_FBO_DIM;
	mP_FboSize			= Vec2f( mP_FboDim, mP_FboDim );
	mP_FboBounds		= Area( 0, 0, mP_FboDim, mP_FboDim );
	mP_PositionFbos[0]	= gl::Fbo( mP_FboDim, mP_FboDim, mRgba16Format );
	mP_PositionFbos[1]	= gl::Fbo( mP_FboDim, mP_FboDim, mRgba16Format );
	mP_VelocityFbos[0]	= gl::Fbo( mP_FboDim, mP_FboDim, mRgba16Format );
	mP_VelocityFbos[1]	= gl::Fbo( mP_FboDim, mP_FboDim, mRgba16Format );
	
	setPredatorFboPositions( mP_PositionFbos[0] );
	setPredatorFboPositions( mP_PositionFbos[1] );
	setPredatorFboVelocities( mP_VelocityFbos[0] );
	setPredatorFboVelocities( mP_VelocityFbos[1] );
	
	initVbo();
	initPredatorVbo();
    
    // AUDIO
    mAudioFboDim    = 16; // 256 bands
    mAudioFboSize   = Vec2f( mAudioFboDim, mAudioFboDim );
    mAudioFboBounds = Area( 0, 0, mAudioFboDim, mAudioFboDim );
    mAudioFbo       = gl::Fbo( mAudioFboDim, mAudioFboDim, mRgba16Format );
}

void Flock::setFboPositions( gl::Fbo fbo )
{	
	// FISH POSITION
	Surface32f posSurface( fbo.getTexture() );
	Surface32f::Iter it = posSurface.getIter();
	while( it.line() ){
		float y = (float)it.y()/(float)it.getHeight() - 0.5f;
		while( it.pixel() ){
			float per		= (float)it.x()/(float)it.getWidth();
			float angle		= per * M_PI * 2.0f;
			float radius	= 100.0f;
			float cosA		= cos( angle );
			float sinA		= sin( angle );
			Vec3f p			= Vec3f( cosA, y, sinA ) * radius;
			
			it.r() = p.x;
			it.g() = p.y;
			it.b() = p.z;
			it.a() = Rand::randFloat( 0.7f, 1.0f );	// GENERAL EMOTIONAL STATE. 
		}
	}
	
	gl::Texture posTexture( posSurface );
	fbo.bindFramebuffer();
	gl::setMatricesWindow( mFboSize, false );
	gl::setViewport( mFboBounds );
	gl::draw( posTexture );
	fbo.unbindFramebuffer();
}

void Flock::setFboVelocities( gl::Fbo fbo )
{
	// FISH VELOCITY
	Surface32f velSurface( fbo.getTexture() );
	Surface32f::Iter it = velSurface.getIter();
	while( it.line() ){
		while( it.pixel() ){
			float per		= (float)it.x()/(float)it.getWidth();
			float angle		= per * M_PI * 2.0f;
			float cosA		= cos( angle );
			float sinA		= sin( angle );
			Vec3f p			= Vec3f( cosA, 0.0f, sinA );
			it.r() = p.x;
			it.g() = p.y;
			it.b() = p.z;
			it.a() = 1.0f;
		}
	}
	
	gl::Texture velTexture( velSurface );
	fbo.bindFramebuffer();
	gl::setMatricesWindow( mFboSize, false );
	gl::setViewport( mFboBounds );
	gl::draw( velTexture );
	fbo.unbindFramebuffer();
}

void Flock::setPredatorFboPositions( gl::Fbo fbo )
{	
	// PREDATOR POSITION
	Surface32f posSurface( fbo.getTexture() );
	Surface32f::Iter it = posSurface.getIter();
	while( it.line() ){
		while( it.pixel() ){
			Vec3f r = Rand::randVec3f() * 50.0f;
			it.r() = r.x;
			it.g() = r.y;
			it.b() = r.z;
			it.a() = Rand::randFloat( 0.7f, 1.0f );	// GENERAL EMOTIONAL STATE. 
		}
	}
	
	gl::Texture posTexture( posSurface );
	fbo.bindFramebuffer();
	gl::setMatricesWindow( mP_FboSize, false );
	gl::setViewport( mP_FboBounds );
	gl::draw( posTexture );
	fbo.unbindFramebuffer();
}

void Flock::setPredatorFboVelocities( gl::Fbo fbo )
{
	// PREDATOR VELOCITY
	Surface32f velSurface( fbo.getTexture() );
	Surface32f::Iter it = velSurface.getIter();
	while( it.line() ){
		while( it.pixel() ){
			Vec3f r = Rand::randVec3f() * 3.0f;
			it.r() = r.x;
			it.g() = r.y;
			it.b() = r.z;
			it.a() = 1.0f;
		}
	}
	
	gl::Texture velTexture( velSurface );
	fbo.bindFramebuffer();
	gl::setMatricesWindow( mP_FboSize, false );
	gl::setViewport( mP_FboBounds );
	gl::draw( velTexture );
	fbo.unbindFramebuffer();
}

void Flock::initVbo()
{
	gl::VboMesh::Layout layout;
	layout.setStaticPositions();
	layout.setStaticTexCoords2d();
	layout.setStaticNormals();
	
	int numVertices = mFboDim * mFboDim;
	// 5 points make up the pyramid
	// 8 triangles make up two pyramids
	// 3 points per triangle
	
	mVboMesh		= gl::VboMesh( numVertices * 8 * 3, 0, layout, GL_TRIANGLES );
	
	float s = 0.5f; // girth
	Vec3f p0( 0.0f, 0.0f, 2.0f );
	Vec3f p1( -s, -s, 0.0f );
	Vec3f p2( -s,  s, 0.0f );
	Vec3f p3(  s,  s, 0.0f );
	Vec3f p4(  s, -s, 0.0f );
	Vec3f p5( 0.0f, 0.0f, -10.0f );
	
	Vec3f n;
	Vec3f n0 = Vec3f( 0.0f, 0.0f, 1.0f );
	Vec3f n1 = Vec3f(-1.0f,-1.0f, 0.0f ).normalized();
	Vec3f n2 = Vec3f(-1.0f, 1.0f, 0.0f ).normalized();
	Vec3f n3 = Vec3f( 1.0f, 1.0f, 0.0f ).normalized();
	Vec3f n4 = Vec3f( 1.0f,-1.0f, 0.0f ).normalized();
	Vec3f n5 = Vec3f( 0.0f, 0.0f,-1.0f );
	
	vector<Vec3f>		positions;
	vector<Vec3f>		normals;
	vector<Vec2f>		texCoords;
	
	for( int x = 0; x < mFboDim; ++x ) {
		for( int y = 0; y < mFboDim; ++y ) {
			float u = (float)x/(float)mFboDim;
			float v = (float)y/(float)mFboDim;
			Vec2f t = Vec2f( u, v );
			
			positions.push_back( p0 );
			positions.push_back( p1 );
			positions.push_back( p2 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p0 + p1 + p2 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p0 );
			positions.push_back( p2 );
			positions.push_back( p3 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p0 + p2 + p3 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p0 );
			positions.push_back( p3 );
			positions.push_back( p4 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p0 + p3 + p4 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p0 );
			positions.push_back( p4 );
			positions.push_back( p1 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p0 + p4 + p1 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			
			
			
			positions.push_back( p5 );
			positions.push_back( p1 );
			positions.push_back( p4 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p5 + p1 + p4 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p5 );
			positions.push_back( p2 );
			positions.push_back( p1 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p5 + p2 + p1 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p5 );
			positions.push_back( p3 );
			positions.push_back( p2 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p5 + p3 + p2 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p5 );
			positions.push_back( p4 );
			positions.push_back( p3 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p5 + p4 + p3 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			
		}
	}
	
	mVboMesh.bufferPositions( positions );
	mVboMesh.bufferTexCoords2d( 0, texCoords );
	mVboMesh.bufferNormals( normals );
	mVboMesh.unbindBuffers();
}

void Flock::initPredatorVbo()
{
	gl::VboMesh::Layout layout;
	layout.setStaticPositions();
	layout.setStaticTexCoords2d();
	layout.setStaticNormals();
	
	int numVertices = mP_FboDim * mP_FboDim;
	// 5 points make up the pyramid
	// 8 triangles make up two pyramids
	// 3 points per triangle
	
	mP_VboMesh		= gl::VboMesh( numVertices * 8 * 3, 0, layout, GL_TRIANGLES );
	
	float s = 1.0f;
	Vec3f p0( 0.0f, 0.0f, 3.0f );
	Vec3f p1( -s*1.3f, 0.0f, 0.0f );
	Vec3f p2( 0.0f, s * 0.5f, 0.0f );
	Vec3f p3( s*1.3f, 0.0f, 0.0f );
	Vec3f p4( 0.0f, -s * 0.5f, 0.0f );
	Vec3f p5( 0.0f, 0.0f, -12.0f );
	

	
	Vec3f n;
	Vec3f n0 = Vec3f( 0.0f, 0.0f, 1.0f );
	Vec3f n1 = Vec3f(-1.0f, 0.0f, 0.0f ).normalized();
	Vec3f n2 = Vec3f( 0.0f, 1.0f, 0.0f ).normalized();
	Vec3f n3 = Vec3f( 1.0f, 0.0f, 0.0f ).normalized();
	Vec3f n4 = Vec3f( 0.0f,-1.0f, 0.0f ).normalized();
	Vec3f n5 = Vec3f( 0.0f, 0.0f,-1.0f );
	
	vector<Vec3f>		positions;
	vector<Vec3f>		normals;
	vector<Vec2f>		texCoords;
	
	for( int x = 0; x < mP_FboDim; ++x ) {
		for( int y = 0; y < mP_FboDim; ++y ) {
			float u = (float)x/(float)mP_FboDim;
			float v = (float)y/(float)mP_FboDim;
			Vec2f t = Vec2f( u, v );
			
			positions.push_back( p0 );
			positions.push_back( p1 );
			positions.push_back( p2 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p0 + p1 + p2 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p0 );
			positions.push_back( p2 );
			positions.push_back( p3 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p0 + p2 + p3 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p0 );
			positions.push_back( p3 );
			positions.push_back( p4 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p0 + p3 + p4 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p0 );
			positions.push_back( p4 );
			positions.push_back( p1 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p0 + p4 + p1 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			
			
			
			positions.push_back( p5 );
			positions.push_back( p1 );
			positions.push_back( p4 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p5 + p1 + p4 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p5 );
			positions.push_back( p2 );
			positions.push_back( p1 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p5 + p2 + p1 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p5 );
			positions.push_back( p3 );
			positions.push_back( p2 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p5 + p3 + p2 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p5 );
			positions.push_back( p4 );
			positions.push_back( p3 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p5 + p4 + p3 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			
		}
	}
	
	mP_VboMesh.bufferPositions( positions );
	mP_VboMesh.bufferTexCoords2d( 0, texCoords );
	mP_VboMesh.bufferNormals( normals );
	mP_VboMesh.unbindBuffers();
}

void Flock::update(double dt)
{
    if( !mInitUpdateCalled )
        mInitUpdateCalled = true;
    
    dt *= (60 * mTimeScale);
    
	// CONTROLLER
	mController.update(dt);
    
    // CAMERA
    if( mCamType == CAM_SPRING )
    {
        if( mMousePressed ){
            mSpringCam.dragCam( ( mMouseOffset ) * 0.01f, ( mMouseOffset ).length() * 0.01f );
        }
        mSpringCam.update( 0.3f );
	}
    else if( mCamType == CAM_SPLINE )
    {
        mSplineCam.update(dt);
    }
    
	gl::disableAlphaBlending();
	gl::disableDepthRead();
	gl::disableDepthWrite();
	gl::color( Color( 1, 1, 1 ) );
    
    // AUDIO
    updateAudioResponse();

	drawIntoVelocityFbo(dt);
	drawIntoPositionFbo(dt);
	drawIntoPredatorVelocityFbo(dt);
	drawIntoPredatorPositionFbo(dt);
	drawIntoLanternsFbo();
}

void Flock::updateAudioResponse()
{
    AudioInput& audioInput = mApp->getAudioInput();
	
    // Get data
    //float * freqData = audioInput.getFft()->getAmplitude();
    //float * timeData = audioInput.getFft()->getData();
    int32_t dataSize = audioInput.getFft()->getBinSize();
    const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
    int32_t index = 0;
    
	Surface32f fftSurface( mAudioFbo.getTexture() );
	Surface32f::Iter it = fftSurface.getIter();
	while( it.line() )
    {
		while( it.pixel() && index < dataSize )
        {
			it.r() = fftLogData[index].y;
            it.g() = 0.0f; // UNUSED
			it.b() = 0.0f; // UNUSED
			it.a() = 1.0f; // UNUSED
            ++index;
		}
	}
	
	gl::Texture fftTexture( fftSurface );
	mAudioFbo.bindFramebuffer();
	gl::setMatricesWindow( mAudioFboSize, false );
	gl::setViewport( mAudioFboBounds );
	gl::draw( fftTexture );
	mAudioFbo.unbindFramebuffer();
}

#pragma mark - Draw

const Camera& Flock::getCamera()
{
    switch( mCamType )
    {
        case CAM_SPRING:
            return mSpringCam.getCam();
            
        case CAM_SPLINE:
            return mSplineCam.getCamera();
            
        case CAM_GRAVITON:
        {
            Scene* gravitonScene = mApp->getScene("graviton");
            
            if( gravitonScene && gravitonScene->isRunning() )
            {
                return gravitonScene->getCamera();
            }
            else
            {
                return mSpringCam.getCam();
            }
        }
            break;
            
        case CAM_ORBITER:
        {
            Scene* orbiterScene = mApp->getScene("orbiter");
            
            if( orbiterScene && orbiterScene->isRunning() )
            {
                return orbiterScene->getCamera();
            }
            else
            {
                return mSpringCam.getCam();
            }
        }
            break;
            
        default:
            return mApp->getMayaCam();
    }
}

// FISH VELOCITY
void Flock::drawIntoVelocityFbo(double dt)
{
	gl::setMatricesWindow( mFboSize, false );
	gl::setViewport( mFboBounds );
	
	mVelocityFbos[ mThisFbo ].bindFramebuffer();
	gl::clear( ColorA( 0, 0, 0, 0 ) );
	
	mPositionFbos[ mPrevFbo ].bindTexture( 0 );
	mVelocityFbos[ mPrevFbo ].bindTexture( 1 );
	mP_PositionFbos[ mPrevFbo ].bindTexture( 2 );
	mLanternsFbo.bindTexture( 3 );
	
	mVelocityShader.bind();
	mVelocityShader.uniform( "positionTex", 0 );
	mVelocityShader.uniform( "velocityTex", 1 );
	mVelocityShader.uniform( "predatorPositionTex", 2 );
	mVelocityShader.uniform( "lanternsTex", 3 );
	mVelocityShader.uniform( "numLights", (float)mController.mNumLanterns );
	mVelocityShader.uniform( "invNumLights", 1.0f/(float)MAX_LANTERNS );
	mVelocityShader.uniform( "invNumLightsHalf", 1.0f/(float)MAX_LANTERNS * 0.5f );
	mVelocityShader.uniform( "att", 1.015f );
	mVelocityShader.uniform( "roomBounds", Vec3f( 350.0f, 200.0f, 350.0f ) );
	mVelocityShader.uniform( "fboDim", mFboDim );
	mVelocityShader.uniform( "invFboDim", 1.0f/(float)mFboDim );
	mVelocityShader.uniform( "pFboDim", mP_FboDim );
	mVelocityShader.uniform( "pInvFboDim", 1.0f/(float)mP_FboDim );
	mVelocityShader.uniform( "dt", (float)dt );
	mVelocityShader.uniform( "power", 1.0f );
	gl::drawSolidRect( mFboBounds );
	mVelocityShader.unbind();
	
	mVelocityFbos[ mThisFbo ].unbindFramebuffer();
}

// FISH POSITION
void Flock::drawIntoPositionFbo(double dt)
{	
	gl::setMatricesWindow( mFboSize, false );
	gl::setViewport( mFboBounds );
	
	mPositionFbos[ mThisFbo ].bindFramebuffer();
	mPositionFbos[ mPrevFbo ].bindTexture( 0 );
	mVelocityFbos[ mThisFbo ].bindTexture( 1 );
	
	mPositionShader.bind();
	mPositionShader.uniform( "position", 0 );
	mPositionShader.uniform( "velocity", 1 );
	mPositionShader.uniform( "dt", (float)dt );
	gl::drawSolidRect( mFboBounds );
	mPositionShader.unbind();
	
	mPositionFbos[ mThisFbo ].unbindFramebuffer();
}

// PREDATOR VELOCITY
void Flock::drawIntoPredatorVelocityFbo(double dt)
{
	gl::setMatricesWindow( mP_FboSize, false );
	gl::setViewport( mP_FboBounds );
	
	mP_VelocityFbos[ mThisFbo ].bindFramebuffer();
	gl::clear( ColorA( 0, 0, 0, 0 ) );
	
	mP_PositionFbos[ mPrevFbo ].bindTexture( 0 );
	mP_VelocityFbos[ mPrevFbo ].bindTexture( 1 );
	mPositionFbos[ mPrevFbo ].bindTexture( 2 );
	mLanternsFbo.bindTexture( 3 );
	mP_VelocityShader.bind();
	mP_VelocityShader.uniform( "positionTex", 0 );
	mP_VelocityShader.uniform( "velocityTex", 1 );
	mP_VelocityShader.uniform( "preyPositionTex", 2 );
	mP_VelocityShader.uniform( "lanternsTex", 3 );
	mP_VelocityShader.uniform( "numLights", (float)mController.mNumLanterns );
	mP_VelocityShader.uniform( "invNumLights", 1.0f/(float)MAX_LANTERNS );
	mP_VelocityShader.uniform( "invNumLightsHalf", 1.0f/(float)MAX_LANTERNS * 0.5f );
	mP_VelocityShader.uniform( "att", 1.015f );
	mP_VelocityShader.uniform( "roomBounds", Vec3f( 350.0f, 200.0f, 350.0f ) );
	mP_VelocityShader.uniform( "fboDim", mP_FboDim );
	mP_VelocityShader.uniform( "invFboDim", 1.0f/(float)mP_FboDim );
	mP_VelocityShader.uniform( "preyFboDim", mFboDim );
	mP_VelocityShader.uniform( "invPreyFboDim", 1.0f/(float)mFboDim );
	
	mP_VelocityShader.uniform( "dt", (float)dt );
	mP_VelocityShader.uniform( "power", 1.0f );
	gl::drawSolidRect( mP_FboBounds );
	mP_VelocityShader.unbind();
	
	mP_VelocityFbos[ mThisFbo ].unbindFramebuffer();
}

// PREDATOR POSITION
void Flock::drawIntoPredatorPositionFbo(double dt)
{
	gl::setMatricesWindow( mP_FboSize, false );
	gl::setViewport( mP_FboBounds );
	
	mP_PositionFbos[ mThisFbo ].bindFramebuffer();
	mP_PositionFbos[ mPrevFbo ].bindTexture( 0 );
	mP_VelocityFbos[ mThisFbo ].bindTexture( 1 );
	
	mP_PositionShader.bind();
	mP_PositionShader.uniform( "position", 0 );
	mP_PositionShader.uniform( "velocity", 1 );
	mP_PositionShader.uniform( "dt", (float)dt );
	gl::drawSolidRect( mP_FboBounds );
	mP_PositionShader.unbind();
	
	mP_PositionFbos[ mThisFbo ].unbindFramebuffer();
}

void Flock::draw()
{
    if( !mInitUpdateCalled )
        return;
    
    //glDisable(GL_TEXTURE_2D);
    gl::pushMatrices();
    
    ////////------------------------------------------------------
    //
	//gl::clear( ColorA( 0.1f, 0.1f, 0.1f, 0.0f ), true );
	gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
	
	gl::setMatricesWindow( mApp->getViewportSize(), false );
	gl::setViewport( getWindowBounds() );
    
    Vec3f billboardRight;
    Vec3f billboardUp;
    getCamera().getBillboardVectors( &billboardRight, &billboardUp );
    
//	gl::disableDepthRead();
//	gl::disableDepthWrite();
//	gl::enable( GL_TEXTURE_2D );
//	gl::enableAlphaBlending();
	
	// DRAW ROOM
	//mRoomFbo.bindTexture();
	//gl::drawSolidRect( getWindowBounds() );
	
	gl::setMatrices( getCamera() );
	gl::setViewport( getWindowBounds() );
	
	gl::enableAlphaBlending();
	gl::enable( GL_TEXTURE_2D );
	gl::enableDepthRead();
	gl::enableDepthWrite();
	
	// DRAW PARTICLES
	mPositionFbos[mPrevFbo].bindTexture( 0 );
	mPositionFbos[mThisFbo].bindTexture( 1 );
	mVelocityFbos[mThisFbo].bindTexture( 2 );
	mLanternsFbo.bindTexture( 3 );
	mShader.bind();
	mShader.uniform( "prevPosition", 0 );
	mShader.uniform( "currentPosition", 1 );
	mShader.uniform( "currentVelocity", 2 );
	mShader.uniform( "lightsTex", 3 );
	mShader.uniform( "numLights", (float)mController.mNumLanterns );
	mShader.uniform( "invNumLights", 1.0f/(float)MAX_LANTERNS );
	mShader.uniform( "invNumLightsHalf", 1.0f/(float)MAX_LANTERNS * 0.5f );
	mShader.uniform( "att", 1.05f );
	mShader.uniform( "eyePos", getCamera().getEyePoint());
	mShader.uniform( "power", 1.0f );
	gl::draw( mVboMesh );
	mShader.unbind();
	
    if (mDrawPredators)
    {
        // DRAW PREDATORS
        mP_PositionFbos[mPrevFbo].bindTexture( 0 );
        mP_PositionFbos[mThisFbo].bindTexture( 1 );
        mP_VelocityFbos[mThisFbo].bindTexture( 2 );
        mLanternsFbo.bindTexture( 3 );
        mP_Shader.bind();
        mP_Shader.uniform( "prevPosition", 0 );
        mP_Shader.uniform( "currentPosition", 1 );
        mP_Shader.uniform( "currentVelocity", 2 );
        mP_Shader.uniform( "lightsTex", 3 );
        mP_Shader.uniform( "numLights", (float)mController.mNumLanterns );
        mP_Shader.uniform( "invNumLights", 1.0f/(float)MAX_LANTERNS );
        mP_Shader.uniform( "invNumLightsHalf", 1.0f/(float)MAX_LANTERNS * 0.5f );
        mP_Shader.uniform( "att", 1.05f );
        mP_Shader.uniform( "eyePos", getCamera().getEyePoint());
        mP_Shader.uniform( "power", 1.0f );
        gl::draw( mP_VboMesh );
        mP_Shader.unbind();
    }
    
	// DRAW LANTERN GLOWS
    gl::disableDepthWrite();
    gl::enableAdditiveBlending();
    float c =  1.0f;
    gl::color( Color( c, c, c ) );
    mLanternGlowTex.bind();
    mController.drawLanternGlows( billboardRight, billboardUp );
    
    drawGlows( billboardRight, billboardUp );
    if( mDrawNebulas )
    {
        drawNebulas( billboardRight, billboardUp );
    }
	
	gl::disable( GL_TEXTURE_2D );
	gl::enableDepthWrite();
	gl::enableAdditiveBlending();
	gl::color( Color( 1.0f, 1.0f, 1.0f ) );
	
	// DRAW LANTERNS
	mLanternShader.bind();
	mLanternShader.uniform( "mvpMatrix", getCamera().getProjectionMatrix() * getCamera().getModelViewMatrix()  );
	mLanternShader.uniform( "eyePos", getCamera().getEyePoint());
	mLanternShader.uniform( "mainPower", 1.0f );
	mLanternShader.uniform( "roomDim", Vec3f( 350.0f, 200.0f, 350.0f ) );
	mController.drawLanterns( &mLanternShader );
	mLanternShader.unbind();

	//gl::disableDepthWrite();
	gl::enableAlphaBlending();
	
	// DRAW PANEL
	//drawInfoPanel();
	
	mThisFbo	= ( mThisFbo + 1 ) % 2;
	mPrevFbo	= ( mThisFbo + 1 ) % 2;
    //
    ////////------------------------------------------------------
    
    gl::popMatrices();
}

void Flock::drawDebug()
{
    gl::pushMatrices();
    
    // DRAW POSITION AND VELOCITY FBOS
    gl::color( Color::white() );
    gl::setMatricesWindow( mApp->getViewportSize() );
    gl::enable( GL_TEXTURE_2D );
    mPositionFbos[ mThisFbo ].bindTexture();
    gl::drawSolidRect( Rectf( 5.0f, 5.0f, 105.0f, 105.0f ) );
    
    mPositionFbos[ mPrevFbo ].bindTexture();
    gl::drawSolidRect( Rectf( 106.0f, 5.0f, 206.0f, 105.0f ) );
    
    mVelocityFbos[ mThisFbo ].bindTexture();
    gl::drawSolidRect( Rectf( 5.0f, 106.0f, 105.0f, 206.0f ) );
    
    mVelocityFbos[ mPrevFbo ].bindTexture();
    gl::drawSolidRect( Rectf( 106.0f, 106.0f, 206.0f, 206.0f ) );
    
    mAudioFbo.bindTexture();
    gl::drawSolidRect( Rectf( 5.0f, 207.0f, 105.0f, 307.0f ) );
    
    gl::popMatrices();
}

void Flock::drawGlows( const Vec3f& billboardRight, const Vec3f& billboardUp )
{
	mGlowTex.bind( 0 );
	mGlowShader.bind();
	mGlowShader.uniform( "glowTex", 0 );
	mGlowShader.uniform( "roomDims", Vec3f( 350.0f, 200.0f, 350.0f ) );
	mController.drawGlows( &mGlowShader, billboardRight, billboardUp );
	mGlowShader.unbind();
}

void Flock::drawNebulas( const Vec3f& billboardRight, const Vec3f& billboardUp )
{
	mNebulaTex.bind( 0 );
	mNebulaShader.bind();
	mNebulaShader.uniform( "nebulaTex", 0 );
	mNebulaShader.uniform( "roomDims", Vec3f( 350.0f, 200.0f, 350.0f ) );
	mController.drawNebulas( &mNebulaShader, billboardRight, billboardUp );
	mNebulaShader.unbind();
}

// HOLDS DATA FOR LANTERNS AND PREDATORS
void Flock::drawIntoLanternsFbo()
{
	Surface32f lanternsSurface( mLanternsFbo.getTexture() );
	Surface32f::Iter it = lanternsSurface.getIter();
	while( it.line() ){
		while( it.pixel() ){
			int index = it.x();
			
			if( it.y() == 0 ){ // set light position
				if( index < (int)mController.mLanterns.size() ){
					it.r() = mController.mLanterns[index].mPos.x;
					it.g() = mController.mLanterns[index].mPos.y;
					it.b() = mController.mLanterns[index].mPos.z;
					it.a() = mController.mLanterns[index].mRadius;
				} else { // if the light shouldnt exist, put it way out there
					it.r() = 0.0f;
					it.g() = 0.0f;
					it.b() = 0.0f;
					it.a() = 1.0f;
				}
			} else {	// set light color
				if( index < (int)mController.mLanterns.size() ){
					it.r() = mController.mLanterns[index].mColor.r;
					it.g() = mController.mLanterns[index].mColor.g;
					it.b() = mController.mLanterns[index].mColor.b;
					it.a() = 1.0f;
				} else { 
					it.r() = 0.0f;
					it.g() = 0.0f;
					it.b() = 0.0f;
					it.a() = 1.0f;
				}
			}
		}
	}

	mLanternsFbo.bindFramebuffer();
	gl::setMatricesWindow( mLanternsFbo.getSize(), false );
	gl::setViewport( mLanternsFbo.getBounds() );
	gl::draw( gl::Texture( lanternsSurface ) );
	mLanternsFbo.unbindFramebuffer();
}

//
////////------------------------------------------------------
#pragma mark - Input

void Flock::handleMouseDown( const MouseEvent& event )
{
    if( event.isRight() ){
        //		mArcball.mouseDown( event.getPos() );
		//mWasRightButtonLastClicked = true;
	} else {
		//mWasRightButtonLastClicked = false;
	}
	//mMouseTimePressed = getElapsedSeconds();
	mMouseDownPos = event.getPos();
	mMousePressed = true;
	mMouseOffset = Vec2f::zero();
    
    Vec3f dims( 350.0f, 200.0f, 350.0f );
    Vec3f randPos( Rand::randFloat( -dims.x * 0.8f, dims.x * 0.8f ),
                  dims.y,
                  Rand::randFloat( -dims.z * 0.5f, dims.z * 0.5f ) );
    //mController.addLantern(randPos);
}

void Flock::handleMouseUp( const MouseEvent& event )
{
	//mMouseTimeReleased	= getElapsedSeconds();
	mMousePressed = false;
	mMouseOffset = Vec2f::zero();
}

void Flock::handleMouseMove( const MouseEvent& event )
{
    mMousePos = event.getPos();
}

void Flock::handleMouseDrag( const MouseEvent& event )
{
	handleMouseMove( event );
	mMouseOffset = ( mMousePos - mMouseDownPos ) * 0.4f;
}

