/*
 *  Corona.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Corona.h"
#include "CubeMap.h"
#include "Star.h"

#include "OculonApp.h"
#include "Interface.h"
#include "Resources.h"

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

using namespace ci;
using namespace ci::app;
using namespace std;

Corona::Corona()
: Scene("corona")
{
}

Corona::~Corona()
{
}

void Corona::setup()
{
    Scene::setup();
    
    // params
    mRadiusAudioResponseBand = AudioInputHandler::BAND_NONE;
    mMinAudioRadius = 0.1f;
	
	// FBOS
	gl::Fbo::Format format;
	format.setColorInternalFormat( GL_RGBA16F_ARB );
	mThisFbo			= 0;
	mPrevFbo			= 1;
	
	// LOAD SHADERS
	try {
		mGradientShader = gl::GlslProg( loadResource( "passThruCorona.vert" ), loadResource( "gradient.frag" ) );
		mStarShader		= gl::GlslProg( loadResource( "star.vert" ), loadResource( "star.frag" ) );
		mGlowShader		= gl::GlslProg( loadResource( "passThruCorona.vert" ), loadResource( "glowCorona.frag" ) );
		mNebulaShader	= gl::GlslProg( loadResource( "passThruCorona.vert" ), loadResource( "nebulaCorona.frag" ) );
		mCoronaShader	= gl::GlslProg( loadResource( "passThruCorona.vert" ), loadResource( "corona.frag" ) );
		mDustShader		= gl::GlslProg( loadResource( "passThruColorCorona.vert" ), loadResource( "dustCorona.frag" ) );
		mPlanetShader	= gl::GlslProg( loadResource( "passThruNormalsCorona.vert" ), loadResource( "planet.frag" ) );
	} catch( gl::GlslProgCompileExc e ) {
		std::cout << e.what() << std::endl;
		exit(1);
	}
	
	// LOAD TEXTURES
	mSpectrumTex		= gl::Texture( loadImage( loadResource( "spectrum.png" ) ) );
	mGlowTex			= gl::Texture( loadImage( loadResource( "glow.png" ) ) );
	mNebulaTex			= gl::Texture( loadImage( loadResource( "nebula.png" ) ) );
	mCoronaTex			= gl::Texture( loadImage( loadResource( "corona.png" ) ) );
	mGridTex			= gl::Texture( loadImage( loadResource( "grid.png" ) ) );
	mSmallGridTex		= gl::Texture( loadImage( loadResource( "smallGrid.png" ) ) );
	mBigGlow0Tex		= gl::Texture( loadImage( loadResource( "bigGlow0.png" ) ) );
	mBigGlow1Tex		= gl::Texture( loadImage( loadResource( "bigGlow1.png" ) ) );
	//mIconTex			= gl::Texture( loadImage( loadResource( "iconStar.png" ) ) );
	mCubeMap			= CubeMap( GLsizei(512), GLsizei(512),
								  Surface8u( loadImage( loadResource( RES_CUBE1_ID ) ) ),
								  Surface8u( loadImage( loadResource( RES_CUBE2_ID ) ) ),
								  Surface8u( loadImage( loadResource( RES_CUBE3_ID ) ) ),
								  Surface8u( loadImage( loadResource( RES_CUBE4_ID ) ) ),
								  Surface8u( loadImage( loadResource( RES_CUBE5_ID ) ) ),
								  Surface8u( loadImage( loadResource( RES_CUBE6_ID ) ) )
								  );
	
	// FONTS
	mFontBlackS			= Font( "Arial", 12 );
	mFontBlackM			= Font( "Arial", 20 );
	mFontBlackL			= Font( "Arial", 64 );
	mTextureFontS		= gl::TextureFont::create( mFontBlackS );
	mTextureFontM		= gl::TextureFont::create( mFontBlackM );
	mTextureFontL		= gl::TextureFont::create( mFontBlackL );
	
	// STAR
	mStar				= CoronaStar( Vec3f::zero(), 4000000.0f );
	createSphere( mStarVbo, 4 );
	createSphere( mStarVboLo, 3 );
	mCanisMajorisPos	= Vec3f( mApp->getViewportWidth(), 0.0f, 0.0f );
	mCanisMajorisPer	= 0.0f;
	
	// MOUSE
	mMousePos			= Vec2f::zero();
	mMouseDownPos		= Vec2f::zero();
	mMouseOffset		= Vec2f::zero();
	mMousePressed		= false;

	// RENDER MODES
	mBillboard			= false;
	mRenderGlows		= false;
	mRenderSphere		= false;
	mRenderNebulas		= false;
	mRenderTexture		= false;
	mRenderDusts		= false;
	mRenderCorona		= false;
	mRenderCanisMajoris = false;
	
	mStage				= EVERYTHING;
	mRandIterations		= 0;
	
	mTitles.push_back( "STAR CREATION" );
	mTitles.push_back( "CIRCLE" );
	mTitles.push_back( "GRADIENT" );
	mTitles.push_back( "CORONAL RING" );
	mTitles.push_back( "SPHERE" );
	mTitles.push_back( "PARTICLE EXPANSION" );
	mTitles.push_back( "PARTICLE ROTATION" );
	mTitles.push_back( "DUST" );
	mTitles.push_back( "STAR" );
	mTitles.push_back( "CONTENT MISSING" );
	
	float solarRadius = mStar.mMaxRadius * 0.25f;
	
	mNumStarTypes		= 8;
	mStarTypeIndex		= 0;
	mStarNames.push_back( "THE SUN" );
	mStarTemps.push_back( 5800 );
	mStarRadii.push_back( solarRadius );
	mStarColors.push_back( 0.75f );
	mStarDists.push_back( 0.0f );
	//mStarPlanets.push_back( 8 );
	
	mStarNames.push_back( "SIRIUS A" );
	mStarTemps.push_back( 9940 );
	mStarRadii.push_back( 1.7f * solarRadius );
	mStarColors.push_back( 0.9f );
	mStarDists.push_back( 8.6f );
	mStarPlanets.push_back( 0 );
	
	mStarNames.push_back( "VEGA" );
	mStarTemps.push_back( 9602 );
	mStarRadii.push_back( 2.2f * solarRadius );
	mStarColors.push_back( 0.95f );
	mStarDists.push_back( 25.04f );
	mStarPlanets.push_back( 0 );
	
	mStarNames.push_back( "GLIESE 581" );
	mStarTemps.push_back( 3500 );
	mStarRadii.push_back( 0.3f * solarRadius );
	mStarColors.push_back( 0.3f );
	mStarDists.push_back( 20.3f );
	mStarPlanets.push_back( 6 );
	
	mStarNames.push_back( "HD 10180" );
	mStarTemps.push_back( 5911 );
	mStarRadii.push_back( 1.2f * solarRadius );
	mStarColors.push_back( 0.5f );
	mStarDists.push_back( 127.0f );
	mStarPlanets.push_back( 9 );
	
	mStarNames.push_back( "GLIESE 710" );
	mStarTemps.push_back( 4250 );
	mStarRadii.push_back( 0.67f * solarRadius );
	mStarColors.push_back( 0.3f );
	mStarDists.push_back( 64.0f );
	mStarPlanets.push_back( 0 );
	
	mStarNames.push_back( "ALPHA CENTAURI A" );
	mStarTemps.push_back( 5790 );
	mStarRadii.push_back( 1.227f * solarRadius );
	mStarColors.push_back( 0.4f );
	mStarDists.push_back( 4.8f );
	mStarPlanets.push_back( 0 );
	
	mStarNames.push_back( "PROXIMA CENTAURI" );
	mStarTemps.push_back( 3000 );
	mStarRadii.push_back( 0.15f * solarRadius );
	mStarColors.push_back( 0.1f );
	mStarDists.push_back( 4.24f );
	mStarPlanets.push_back( 0 );
	
	setStage( mStage );
    
    mCameraController.setup(mApp, this, CameraController::CAM_SPRING, CameraController::CAM_SPRING);
    mAudioInputHandler.setup();
    
    mTimeController.setTimeScaleMultiplier(60.0f);
}

// ----------------------------------------------------------------
//
void Corona::setupInterface()
{
    mTimeController.setupInterface(mInterface, mName);
    
    vector<string> bandNames = mAudioInputHandler.getBandNames();
    mInterface->addEnum(CreateEnumParam( "audio-radius", (int*)(&mRadiusAudioResponseBand) )
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(mName)
                        .sendFeedback(), bandNames);
    mInterface->addParam(CreateFloatParam("min_radius_ratio", &mMinAudioRadius)
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam("radius", &mStar.mRadiusDest)
                         .maxValue(mStar.mMaxRadius)
                         .oscReceiver(mName));
    
    // TODO: refactor
    mInterface->addParam(CreateIntParam("stage", &mStage)
                         .maxValue(STAGE_COUNT));
    
    mInterface->addParam(CreateIntParam("star_type", &mStarTypeIndex)
                         .maxValue(mNumStarTypes)
                         .oscReceiver(mName))->registerCallback(this, &Corona::onStarTypeChanged);
    mInterface->addParam(CreateFloatParam("radius", &mStar.mRadiusDest)
                         .maxValue(mStar.mMaxRadius)
                         .oscReceiver(mName));
    
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("-rendering-");
    mInterface->addParam(CreateBoolParam("billboard", &mBillboard)
                         .oscReceiver(mName));
    mInterface->addParam(CreateBoolParam("glows", &mRenderGlows)
                         .oscReceiver(mName));
    mInterface->addParam(CreateBoolParam("nebulas", &mRenderNebulas)
                         .oscReceiver(mName));
    mInterface->addParam(CreateBoolParam("sphere", &mRenderSphere)
                         .oscReceiver(mName));
    mInterface->addParam(CreateBoolParam("texture", &mRenderTexture)
                         .oscReceiver(mName));
    mInterface->addParam(CreateBoolParam("dust", &mRenderDusts)
                         .oscReceiver(mName));
    mInterface->addParam(CreateBoolParam("corona", &mRenderCorona)
                         .oscReceiver(mName));
    mInterface->addParam(CreateBoolParam("canismajoris", &mRenderCanisMajoris)
                         .oscReceiver(mName));
    
    mCameraController.setupInterface(mInterface, mName);
    
    mAudioInputHandler.setupInterface(mInterface, mName);
}

bool Corona::onStarTypeChanged()
{
    setStar(0);
    return true;
}

// ----------------------------------------------------------------
//
//void Corona::setupDebugInterface()
//{
//}
// ----------------------------------------------------------------
//
void Corona::reset()
{
    mTimeController.reset();
}

// ----------------------------------------------------------------
//
void Corona::resize()
{
    Scene::resize();
}

void Corona::createSphere( gl::VboMesh &vbo, int res )
{
	float X = 0.525731112119f; 
	float Z = 0.850650808352f;
	
	static Vec3f verts[12] = {
		Vec3f( -X, 0.0f, Z ), Vec3f( X, 0.0f, Z ), Vec3f( -X, 0.0f, -Z ), Vec3f( X, 0.0f, -Z ),
		Vec3f( 0.0f, Z, X ), Vec3f( 0.0f, Z, -X ), Vec3f( 0.0f, -Z, X ), Vec3f( 0.0f, -Z, -X ),
		Vec3f( Z, X, 0.0f ), Vec3f( -Z, X, 0.0f ), Vec3f( Z, -X, 0.0f ), Vec3f( -Z, -X, 0.0f ) };
	
	static GLuint triIndices[20][3] = { 
		{0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1}, {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},
		{7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6}, {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };
	
	gl::VboMesh::Layout layout;
	layout.setStaticPositions();
	layout.setStaticNormals();
	
	mPosCoords.clear();
	mNormals.clear();

	for( int i=0; i<20; i++ ){
		drawSphereTri( verts[triIndices[i][0]], verts[triIndices[i][1]], verts[triIndices[i][2]], res );
	}
	vbo = gl::VboMesh( mPosCoords.size(), 0, layout, GL_TRIANGLES );	
	vbo.bufferPositions( mPosCoords );
	vbo.bufferNormals( mNormals );
	vbo.unbindBuffers();
}

void Corona::drawSphereTri( Vec3f va, Vec3f vb, Vec3f vc, int div )
{
	if( div <= 0 ){
		mPosCoords.push_back( va );
		mPosCoords.push_back( vb );
		mPosCoords.push_back( vc );
		//Vec3f vn = ( va + vb + vc ) * 0.3333f;
		mNormals.push_back( va );
		mNormals.push_back( vb );
		mNormals.push_back( vc );
	} else {
		Vec3f vab = ( ( va + vb ) * 0.5f ).normalized();
		Vec3f vac = ( ( va + vc ) * 0.5f ).normalized();
		Vec3f vbc = ( ( vb + vc ) * 0.5f ).normalized();
		drawSphereTri( va, vab, vac, div-1 );
		drawSphereTri( vb, vbc, vab, div-1 );
		drawSphereTri( vc, vac, vbc, div-1 );
		drawSphereTri( vab, vbc, vac, div-1 );
	}
}



void Corona::handleMouseDown( const MouseEvent& event )
{
	mMouseDownPos = event.getPos();
	mMousePressed = true;
	mMouseOffset = Vec2f::zero();
}

void Corona::handleMouseUp( const MouseEvent& event )
{
	mMousePressed = false;
	mMouseOffset = Vec2f::zero();
}

void Corona::handleMouseMove( const MouseEvent& event )
{
	mMousePos = event.getPos();
}

void Corona::handleMouseDrag( const MouseEvent& event )
{
	handleMouseMove( event );
	mMouseOffset = ( mMousePos - mMouseDownPos );
}

bool Corona::handleKeyDown( const KeyEvent& event )
{
	switch( event.getChar() ) {
		case '0':	setStage( 0 );				break;
		case '1':	setStage( 1 );				break;
		case '2':	setStage( 2 );				break;
		case '3':	setStage( 3 );				break;
		case '4':	setStage( 4 );				break;
		case '5':	setStage( 5 );				break;
		case '6':	setStage( 6 );				break;
		case '7':	setStage( 7 );				break;
		case 'E':	setStage( 8 );				break;
		case 'D':	setStage( 9 );				break;
		case 'b':	mBillboard		= !mBillboard;		break;
		case 'g':	mRenderGlows	= !mRenderGlows;	break;
		case 'n':	mRenderNebulas	= !mRenderNebulas;	break;
		case 's':	mRenderSphere	= !mRenderSphere;	break;
		case 't':	mRenderTexture	= !mRenderTexture;	break;
		case 'd':	mRenderDusts	= !mRenderDusts;	break;
		case 'c':	mRenderCorona	= !mRenderCorona;	break;
		case 'm':	toggleCanisMajoris();				break;
		case 'T':	randSeed(); mRandIterations ++;		break;
		case ']':	mStar.adjustRadius( 3.0f );		break;
		case '[':	mStar.adjustRadius(-3.0f );		break;
		case '=':	setStar( 1 );					break;
		case '-':	setStar( -1 );					break;
		default:									break;
	}
    
    return false;
}

void Corona::toggleCanisMajoris()
{
	mRenderCanisMajoris = !mRenderCanisMajoris; 
}

void Corona::setStar( int offset )
{
	mStarTypeIndex += offset;
	mStarTypeIndex = constrain( mStarTypeIndex, 0, mNumStarTypes - 1 );
	
	mStar.setRadius( mStarRadii[ mStarTypeIndex ] );
	mStar.setColor( mStarColors[ mStarTypeIndex ] );
	mStar.setNumPlanets( mStarPlanets[ mStarTypeIndex ] );
}

void Corona::setStage( int i )
{
	mStage		= i;
	if( mStage == Corona::CIRCLE ){
		mStar.resetRadius();
	}
	if( mStage == Corona::GRADIENT ){
		mRandIterations ++;
		randSeed();
	}
	
	
	//INTRO, BASIC_SHAPE, GRADIENT, BASE_SPHERE, SPHERE_AND_GLOW, NEBULAS, GLOWS;
	if( mStage == Corona::INTRO ){
		mRenderGlows	= false;
		mRenderNebulas	= false;
		mRenderSphere	= false;
		mRenderCorona	= false;
		mRenderTexture	= false;
		mRenderDusts	= false;
	} else if( mStage == Corona::CIRCLE ){
		mRenderGlows	= false;
		mRenderNebulas	= false;
		mRenderSphere	= false;
		mRenderCorona	= false;
		mRenderTexture	= true;
		mRenderDusts	= false;
	} else if( mStage == Corona::GRADIENT ){
		mRenderGlows	= false;
		mRenderNebulas	= false;
		mRenderSphere	= false;
		mRenderCorona	= false;
		mRenderTexture	= true;
		mRenderDusts	= false;
	} else if( mStage == Corona::CORONA ){
		mRenderGlows	= false;
		mRenderNebulas	= false;
		mRenderSphere	= false;
		mRenderCorona	= true;
		mRenderTexture	= false;
		mRenderDusts	= false;
	} else if( mStage == Corona::SPHERE ){
		mRenderGlows	= false;
		mRenderNebulas	= false;
		mRenderSphere	= true;
		mRenderCorona	= false;
		mRenderTexture	= false;
		mRenderDusts	= false;
	} else if( mStage == Corona::NEBULAS ){
		mRenderGlows	= false;
		mRenderNebulas	= true;
		mRenderSphere	= false;
		mRenderCorona	= false;
		mRenderTexture	= false;
		mRenderDusts	= false;
	} else if( mStage == Corona::GLOWS ){
		mRenderGlows	= true;
		mRenderNebulas	= false;
		mRenderSphere	= false;
		mRenderCorona	= false;
		mRenderTexture	= false;
		mRenderDusts	= false;
	} else if( mStage == Corona::DUSTS ){
		mRenderGlows	= false;
		mRenderNebulas	= false;
		mRenderSphere	= false;
		mRenderCorona	= false;
		mRenderTexture	= false;
		mRenderDusts	= true;
	} else if( mStage == Corona::EVERYTHING ){
		mRenderGlows	= true;
		mRenderNebulas	= true;
		mRenderSphere	= true;
		mRenderCorona	= true;
		mRenderTexture	= true;
		mRenderDusts	= true;
		mBillboard		= true;
	} else if( mStage == Corona::LESSON ){
		mRenderGlows	= true;
		mRenderNebulas	= true;
		mRenderSphere	= true;
		mRenderCorona	= true;
		mRenderTexture	= true;
		mRenderDusts	= true;
		//mSpringCam.mEyeNode.mRestPos = Vec3f( 0.0f, 80.0f, -466.0f );
		mBillboard		= true;
		setStar( 0 );
	}
}

void Corona::randSeed()
{
	mRandSeed	= Rand::randVec2f() * Rand::randFloat( 100.0f );
}

void Corona::update(double dt)
{
    mTimeController.update(dt);
	mCameraController.update(dt);
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    float simDt = mTimeController.getDelta();
    
    if (mRadiusAudioResponseBand != AudioInputHandler::BAND_NONE)
    {
        mStar.mRadius = mStarRadii[ mStarTypeIndex ] * (mAudioInputHandler.getAverageVolumeByBand(mRadiusAudioResponseBand) + mMinAudioRadius);
        mStar.mRadiusDest = mStar.mRadius;
    }
    
	if( mRenderCanisMajoris )
    {
		mCanisMajorisPer -= ( mCanisMajorisPer - 1.0f ) * 0.1f * simDt;
		mCanisMajorisPos.x -= mCanisMajorisPos.x * 0.1f * simDt;
	}
    else
    {
		mCanisMajorisPer -= ( mCanisMajorisPer - 0.0f ) * 0.1f * simDt;
		mCanisMajorisPos.x -= ( mCanisMajorisPos.x - mBigGlow0Tex.getWidth() ) * 0.1f * simDt;
	}

	// STAR
	mStar.update( simDt );
	
	// CONTROLLER
	if( mTimeController.getTick() ){
		// ADD GLOWS
		int numGlowsToSpawn = 24;//* mStar.mRadiusMulti );
		mController.addGlows( mStar, 1.0f, numGlowsToSpawn );
		
		// ADD NEBULAS
		int numNebulasToSpawn = 8;//* mStar.mRadiusMulti );
		mController.addNebulas( mStar, numNebulasToSpawn );
		
		// ADD DUSTS
		int numDustsToSpawn = 500;
		mController.addDusts( mStar, numDustsToSpawn );
		
		
		if( mCanisMajorisPer > 0.01f )
        {
			float radius = 4000.0f;
			Vec3f pos = Vec3f( radius + 350.0f, 0.0f, 0.0f ) + mCanisMajorisPos * 0.5f;
			int amt = 24;
			// nebulas off canis majoris
			mController.addCMNebulas( pos, radius, 3.0f, amt );
			
			mController.addCMGlows( pos, radius, amt/2 );
			
			// nebulas at home star
			mController.addCMNebulas( Vec3f::zero(), 0.0f, 1.0f, 1 );
		}
	}
	mController.update( simDt );
	
}

void Corona::draw()
{
	//gl::clear( ColorA( 0, 0, 0, 0 ), true );
    gl::pushMatrices();
	gl::setMatricesWindow( mApp->getViewportSize(), false );
	gl::setViewport( mApp->getViewportBounds() );

	gl::disableDepthRead();
	gl::disableDepthWrite();
	gl::enableAlphaBlending();
	gl::enable( GL_TEXTURE_2D );
	gl::color( ColorA( 1, 1, 1, 1 ) );
	
	gl::enableAdditiveBlending();
	
	if( mCanisMajorisPer > 0.01f ){
		drawCanisMajoris();
	}
	
	gl::setMatrices( getCamera() );
	gl::enableAlphaBlending();	

	gl::enable( GL_TEXTURE_2D );
	gl::enableDepthRead();
	gl::enableDepthWrite();
	
	// DRAW SPHERE
	if( mRenderSphere ){
		drawSphere();
	}
	
	gl::disableDepthWrite();
	
	// DRAW FLAT IMAGES
	if( mRenderTexture ){
		drawFlat();
	}
	
	gl::enableAdditiveBlending();
	
	// DRAW CORONA
	if( mRenderCorona ){
		drawCorona();
	}
	
	gl::enableAdditiveBlending();
	gl::enable( GL_TEXTURE_2D );
	
	gl::color( ColorA( 1, 1, 1, 1 ) );	
	
	// DRAW GLOWS
	if( mRenderGlows ){
		drawGlows();
	}
	
	// DRAW NEBULAS
	if( mRenderNebulas ){
		drawNebulas();
	}
	
	gl::disable( GL_TEXTURE_2D );
	
	// DRAW DUST
	if( mRenderDusts ){
		drawDusts();
	}
	
	gl::enableAdditiveBlending();
	
	// DRAW ORBIT RINGS
	//Color c = Color( 0.1f, 0.2f, 0.6f );
	//gl::color( ColorA( c * 1.0f, 0.4f ) );
	//mStar.drawOrbitRings();
	
	gl::enableAlphaBlending();
	gl::enableDepthWrite();
	
	// DRAW PLANETS
    /*
	mSpectrumTex.bind( 0 );
	gl::color( ColorA( 1, 1, 1, 1 ) );
	mPlanetShader.bind();
	mPlanetShader.uniform( "spectrumTex", 0 );
	mPlanetShader.uniform( "starColor", mStar.mColor );
	mPlanetShader.uniform( "power", 1.0f );
	mPlanetShader.uniform( "windowDims", Vec2f( mApp->getViewportWidth(), mApp->getViewportHeight() ) );
	mPlanetShader.uniform( "eyePos", mSpringCam.getEye() );
	mStar.drawPlanets( &mPlanetShader );
	mPlanetShader.unbind();
     */
	
	mThisFbo	= ( mThisFbo + 1 ) % 2;
	mPrevFbo	= ( mThisFbo + 1 ) % 2;
    
    gl::popMatrices();
}

void Corona::drawSphere()
{
	gl::color( ColorA( 1, 1, 1, 1 ) );
	gl::disable( GL_TEXTURE_2D );
	mCubeMap.bind();
	mSpectrumTex.bind( 1 );
	mStarShader.bind();
	mStarShader.uniform( "cubeMap", 0 );
	mStarShader.uniform( "spectrumTex", 1 );
	mStarShader.uniform( "color", mStar.mColor );
	mStarShader.uniform( "time", (float)mTimeController.getElapsedSeconds() );
	mStarShader.uniform( "radius", mStar.mRadiusDrawn );
	mStarShader.uniform( "mvpMatrix", getCamera().getProjectionMatrix() * getCamera().getModelViewMatrix() );
	mStarShader.uniform( "eyePos", getCamera().getEyePoint() );
	mStarShader.uniform( "mainPower", 1.0f );
	gl::draw( mStarVbo );
	mStarShader.unbind();
}

void Corona::drawCorona()
{
	if( mBillboard ){
		gl::pushMatrices();
		gl::rotate( getCamera().getOrientation() );
	}
	gl::color( ColorA( 1, 1, 1, 1 ) );
	float radius = mStar.mRadius * 1.8f;

	mCoronaTex.bind();
	mSpectrumTex.bind( 1 );
	mCoronaShader.bind();
	mCoronaShader.uniform( "coronaTex", 0 );
	mCoronaShader.uniform( "spectrumTex", 1 );
	mCoronaShader.uniform( "starColor", mStar.mColor );
	mCoronaShader.uniform( "power", 1.0f );
	gl::drawSolidRect( Rectf( -radius, -radius, radius, radius ) );
	mCoronaShader.unbind();
	
	if( mBillboard ) gl::popMatrices();
}
								 
								 

void Corona::drawFlat()
{
	if( mBillboard ){
		gl::pushMatrices();
		gl::rotate( getCamera().getOrientation() );
	}
	gl::color( ColorA( 1, 1, 1, 1 ) );
	float radius = mStar.mRadius;
	if( mStage == CIRCLE ){
		radius = mStar.mRadius;
	} else if( mStage > CIRCLE ){
		radius *= ( 5.0f + 1.0f );
	}
	

	mGridTex.bind( 0 );
	mSpectrumTex.bind( 1 );
	mGradientShader.bind();
	mGradientShader.uniform( "gridTex", 0 );
	mGradientShader.uniform( "spectrumTex", 1 );
	mGradientShader.uniform( "color", mStar.mColor );
	mGradientShader.uniform( "radius", radius );
	mGradientShader.uniform( "starRadius", mStar.mRadiusDrawn );
	mGradientShader.uniform( "power", 1.0f );
	mGradientShader.uniform( "time", (float)getElapsedSeconds() );
	mGradientShader.uniform( "roomDim", 320.0f ); // TODO: what does this affect?
	mGradientShader.uniform( "randIterations", mRandIterations );
	mGradientShader.uniform( "stage", mStage );

		mGradientShader.uniform( "randSeed", mRandSeed );
		mGradientShader.uniform( "unit", 0 );
		gl::drawSolidRect( Rectf( -radius, -radius, radius, radius ) );
	
		mGradientShader.uniform( "randSeed", mRandSeed * 1.5f );
		mGradientShader.uniform( "unit", 2 );
		gl::drawSolidRect( Rectf( -radius, -radius, radius, radius ) );

	mGradientShader.unbind();
	if( mBillboard ) gl::popMatrices();
}

void Corona::drawGlows()
{
	mGlowTex.bind( 0 );
	mSmallGridTex.bind( 1 );
	mSpectrumTex.bind( 2 );
	mGlowShader.bind();
	mGlowShader.uniform( "glowTex", 0 );
	mGlowShader.uniform( "gridTex", 1 );
	mGlowShader.uniform( "spectrumTex", 2 );
	mGlowShader.uniform( "color", mStar.mColor );
	mGlowShader.uniform( "power", 1.0f );
	mGlowShader.uniform( "starRadius", mStar.mRadiusDrawn );
	Vec3f right = Vec3f::xAxis();
	Vec3f up	= Vec3f::yAxis();
	if( mBillboard ) getCamera().getBillboardVectors( &right, &up );

	mController.drawGlows( &mGlowShader, right, up );
	mGlowShader.unbind();
}

void Corona::drawNebulas()
{
	mNebulaTex.bind( 0 );
	mSmallGridTex.bind( 1 );
	mSpectrumTex.bind( 2 );
	mNebulaShader.bind();
	mNebulaShader.uniform( "nebulaTex", 0 );
	mNebulaShader.uniform( "gridTex", 1 );
	mNebulaShader.uniform( "spectrumTex", 2 );
	mNebulaShader.uniform( "color", mStar.mColor );
	mNebulaShader.uniform( "power", 1.0f );
	mNebulaShader.uniform( "starRadius", mStar.mRadiusDrawn );
	Vec3f right = Vec3f::xAxis();
	Vec3f up	= Vec3f::yAxis();
	if( mBillboard ) getCamera().getBillboardVectors( &right, &up );
	
	mController.drawNebulas( &mNebulaShader, right, up );
	mNebulaShader.unbind();
}

void Corona::drawDusts()
{
	gl::pushModelView();
	float per = 1.75f * mStar.mRadius/mStar.mMaxRadius;
	gl::scale( Vec3f( per, per, per ) );
	mSpectrumTex.bind();
	mDustShader.bind();
	mDustShader.uniform( "spectrumTex", 0 );
	mDustShader.uniform( "color", mStar.mColor );
	mDustShader.uniform( "power", 1.0f );
	mController.drawDusts();
	mDustShader.unbind();
	gl::popModelView();
}

void Corona::drawCanisMajoris()
{
	Rectf rect = Rectf( mCanisMajorisPos.x, 0.0f, mCanisMajorisPos.x + mBigGlow0Tex.getWidth(), mApp->getViewportHeight() );
	
	mBigGlow0Tex.bind();
	gl::drawSolidRect( rect );
	
	mBigGlow1Tex.bind();
	gl::drawSolidRect( rect );
}

const Camera& Corona::getCamera()
{
    return mCameraController.getCamera();
}