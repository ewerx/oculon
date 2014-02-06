/*
 *  Catalog.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Catalog.h"

#include "OculonApp.h"
#include "AudioInput.h"
#include "Interface.h"
#include "Resources.h"
#include "Planet.h"

#include "Orbiter.h"

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

using namespace ci;
using namespace ci::app;
using namespace boost;
using namespace std;

const int Catalog::FBO_WIDTH = 78;
const int Catalog::FBO_HEIGHT = 78;

#define USE_NEW_DATA 1

// ----------------------------------------------------------------
//
Catalog::Catalog()
: Scene("catalog")
{
    /*
    for( int i=0; i < TB_COUNT; ++i )
    {
        mTextBox[i] = new TextEntity(this);
        assert(mTextBox[i] != NULL && "out of memory, how ridiculous");
    }
    */
}

// ----------------------------------------------------------------
//
Catalog::~Catalog()
{
    BOOST_FOREACH( Star* &s, mStars ){
		delete s;
	}
}

// ----------------------------------------------------------------
//
void Catalog::setup()
{
    Scene::setup();
    
    // params
    mShowSol = false;
    mMoreGlow = true;
    mCamType = CAM_STAR;
    mCameraDistance = 0.0f;
    
    mLabelBrightnessByAudio = 0.16f;
    
    mNextKeplerIndex = 0;
    mStarCamTimeScale = 0.0025;
    mStarfieldAlpha = 1.0f;
    mNamesAlpha = 1.0f;
    
    // assets
    
    ////////------------------------------------------------------
    //
    // CAMERA	
	mSpringCam		= SpringCam( -350.0f, mApp->getViewportAspectRatio(), 200000.0f );
    mStarCam.setup(mApp);
    //TODO: hack!
    mStarCam.mTimeScale = &mStarCamTimeScale;
    
	// LOAD SHADERS
	try {
		mBrightStarsShader	= gl::GlslProg( loadResource( RES_BRIGHT_STARS_VERT ), loadResource( RES_BRIGHT_STARS_FRAG ) );
		mFaintStarsShader	= gl::GlslProg( loadResource( RES_FAINT_STARS_VERT ), loadResource( RES_FAINT_STARS_FRAG ) );
	} catch( gl::GlslProgCompileExc e ) {
		std::cout << e.what() << std::endl;
		mApp->quit();
	}
	
	// TEXTURE FORMAT
	gl::Texture::Format mipFmt;
    mipFmt.enableMipmapping( true );
    mipFmt.setMinFilter( GL_LINEAR_MIPMAP_LINEAR );    
    mipFmt.setMagFilter( GL_LINEAR );
	
	// LOAD TEXTURES
	mMilkyWayTex	= gl::Texture( loadImage( loadResource( RES_CATALOG_MILKYWAY ) ) );
	mStarTex		= gl::Texture( loadImage( loadResource( RES_CATALOG_STAR ) ), mipFmt );
	mStarGlowTex	= gl::Texture( loadImage( loadResource( RES_CATALOG_STARGLOW ) ), mipFmt );
	mDarkStarTex	= gl::Texture( loadImage( loadResource( RES_CATALOG_DARKSTAR ) ) );
	mSpectrumTex	= gl::Texture( loadImage( loadResource( RES_CATALOG_SPECTRUM ) ) );
	
	// FONTS
	mFontBlackT		= Font( "Menlo", 8 );
	mFontBlackS		= Font( "Menlo", 12 );
	//mTextureFontT	= gl::TextureFont::create( mFontBlackT );
	//mTextureFontS	= gl::TextureFont::create( mFontBlackS );
	
	// MOUSE
	mMousePos		= Vec2f::zero();
	mMouseDownPos	= Vec2f::zero();
	mMouseOffset	= Vec2f::zero();
	mMousePressed	= false;
	mMouseTimePressed	= 0.0f;
	mMouseTimeReleased	= 0.0f;
	mMouseTimeThresh	= 0.2f;
	mWasRightButtonLastClicked = false;
    
	// FBOS
	gl::Fbo::Format format;
	format.setColorInternalFormat( GL_RGBA32F_ARB );
	format.setMinFilter( GL_NEAREST );
	format.setMagFilter( GL_NEAREST );
	mPositionFbo = gl::Fbo( FBO_WIDTH, FBO_HEIGHT, format );
	
	// STARS
	mScaleDest	= 0.2f;
	mScale		= 0.2f;
	mMaxScale	= 400.0f;
	mScalePer	= mScale/mMaxScale;
    mHomeStar	= NULL;
	mDestStar	= NULL;
    mSol        = NULL;
	parseStarData( App::getResourcePath( "starData.csv" ) );
    parsePlanetData( App::getResourcePath( "ExoplanetArchive.csv" ) );
    
	setFboPositions( mPositionFbo );
	
	initFaintVbo();
	initBrightVbo();
	
	mDataTimer	= 0.0f;
	
	// RENDER OPTIONS
	mRenderNames			= true;
	mRenderFaintStars		= true;
	mRenderBrightStars		= true;
    //
    ///////--------------------------------------------------------
    
    reset();
}

// ----------------------------------------------------------------
//
void Catalog::setupInterface()
{
    mInterface->gui()->addColumn();
    vector<string> camTypeNames;
#define CATALOG_CAMTYPE_ENTRY( nam, enm ) \
camTypeNames.push_back(nam);
    CATALOG_CAMTYPE_TUPLE
#undef  CATALOG_CAMTYPE_ENTRY
    mInterface->addEnum(CreateEnumParam( "Cam Type", (int*)(&mCamType) )
                        .maxValue(CAM_COUNT)
                        .oscReceiver(getName(), "camtype")
                        .isVertical(), camTypeNames);
    mInterface->addParam(CreateBoolParam( "show names", &mRenderNames )
                        .oscReceiver(getName(), "shownames"));
    mInterface->addParam(CreateBoolParam( "show sol", &mShowSol )
                         .oscReceiver(getName(), "showsol"));
    mInterface->addParam(CreateBoolParam( "show faint", &mRenderFaintStars )
                         .oscReceiver(getName(), "showfaint"));
    mInterface->addParam(CreateBoolParam( "show bright", &mRenderBrightStars )
                         .oscReceiver(getName(), "showbright"));
    mInterface->addParam(CreateBoolParam( "more glow", &mMoreGlow )
                         .oscReceiver(getName(), "moreglow"));
    /*
    mInterface->addParam(CreateFloatParam("BPM", &mBpm)
                         .minValue(60.0f)
                         .maxValue(150.0f)
                         .oscReceiver(mName,"bpm")
                         .sendFeedback());
    */
    mInterface->addButton(CreateTriggerParam("Random Home", NULL)
                          .oscReceiver(mName,"randhome"))->registerCallback( this, &Catalog::setRandomHome );
    mInterface->addButton(CreateTriggerParam("Random Dest", NULL)
                          .oscReceiver(mName,"randdest"))->registerCallback( this, &Catalog::setRandomDest );
    mInterface->addButton(CreateTriggerParam("Sol Home", NULL)
                          .oscReceiver(mName,"solhome"))->registerCallback( this, &Catalog::setSolHome );
    mInterface->addButton(CreateTriggerParam("Sol Dest", NULL)
                          .oscReceiver(mName,"soldest"))->registerCallback( this, &Catalog::setSolDest );
    mInterface->addButton(CreateTriggerParam("Next Kepler Star", NULL)
                          .oscReceiver(mName,"nextkepler"))->registerCallback( this, &Catalog::setNextKeplerStar );
    mInterface->addButton(CreateTriggerParam("Prev Kepler Star", NULL)
                          .oscReceiver(mName,"prevkepler"))->registerCallback( this, &Catalog::setPrevKeplerStar );
    
    mInterface->addParam(CreateIntParam("Kepler Star Index", &mNextKeplerIndex)
                         .maxValue( mStarsWithPlanets.size() )
                         .oscReceiver(mName,"keplerindex")
                         .sendFeedback())->registerCallback( this, &Catalog::setKeplerStar );
    
    mInterface->addParam(CreateFloatParam("Audio Label Multi", &mLabelBrightnessByAudio)
                         .minValue(0.0f)
                         .maxValue(1.0f)
                         .oscReceiver(mName,"audiolabelmulti")
                         .sendFeedback());
    
    mInterface->addParam(CreateFloatParam("Star Cam Time Scale", &mStarCamTimeScale)
                         .minValue(0.0f)
                         .maxValue(0.01f)
                         .oscReceiver(mName,"starcamtimescale")
                         .sendFeedback());
    
    mInterface->addParam(CreateFloatParam("Star Cam Time x100", &mStarCamTimeScale)
                         .minValue(0.0f)
                         .maxValue(1.0f)
                         .oscReceiver(mName,"starcamtimescale100"));
    
    mInterface->addParam(CreateFloatParam("Starfield Alpha", &mStarfieldAlpha)
                         .minValue(0.0f)
                         .maxValue(1.0f)
                         .oscReceiver(mName,"starfieldalpha")
                         .sendFeedback());
    
    mInterface->addParam(CreateFloatParam("Names Alpha", &mNamesAlpha)
                         .minValue(0.0f)
                         .maxValue(1.0f)
                         .oscReceiver(mName,"namesalpha")
                         .sendFeedback());
}

// ----------------------------------------------------------------
//
void Catalog::setupDebugInterface()
{
    Scene::setupDebugInterface();
    //mDebugParams.addParam("Camera Distance", &mCameraDistance, "readonly=true");
}

// ----------------------------------------------------------------
//
void Catalog::reset()
{
    
}

// ----------------------------------------------------------------
//
void Catalog::resize()
{
    Scene::resize();
//    for( int i = 0; i < TB_COUNT; ++i )
//    {
//        mTextBox[i]->resize();
//    }
}

// ----------------------------------------------------------------
//
void Catalog::update(double dt)
{
    ////////------------------------------------------------------
    //
    if( mCamType == CAM_SPRING && getElapsedSeconds() - mMouseTimePressed < mMouseTimeThresh && !mMousePressed ){
		mMouseTimePressed = 0.0f;
		selectStar( mWasRightButtonLastClicked );
	}
    
    updateAudioResponse();
	
	//mRoom.update();
	
	mScaleDest -= ( mScaleDest - mMaxScale ) * 0.05f;
	mScale -= ( mScale - mScaleDest ) * 0.02f;
	mScalePer = mScale/mMaxScale;
	
	// CAMERA
    if( mCamType == CAM_STAR )
    {
        mStarCam.update(dt);
    }
    
    // convert from parsecs to lightyears
    mCameraDistance = getCamera().getEyePoint().length() * 3.261631f;
    //console() << "[catalog] cam distance = " << mCameraDistance << std::endl;
    
    if( mHomeStar != NULL ){
        mSpringCam.setEye( mHomeStar->mPos + Vec3f( 100.0f, 0.0f, 40.0f ) );
        mStarCam.setCurrentCam(mSpringCam.getCam());
    }
    
    if( mDestStar != NULL ){
        mSpringCam.setCenter( mDestStar->mPos );
        mStarCam.setTarget( mDestStar->mPos );
        mStarCam.setCurrentCam(mSpringCam.getCam());
    }
    
	if( mMousePressed )
		mSpringCam.dragCam( ( mMouseOffset ) * 0.01f, ( mMouseOffset ).length() * 0.01 );
	mSpringCam.update( 0.25f );
	
	BOOST_FOREACH( Star* &s, mBrightStars ){
		s->update( getCamera(), mScale );
	}
    //
    ////////------------------------------------------------------
    
    // last
    Scene::update(dt);
}

// ----------------------------------------------------------------
//
void Catalog::draw()
{
    
    gl::pushMatrices();
    
    ////////------------------------------------------------------
	gl::color( ColorA( 1, 1, 1, 1 ) );
    
	gl::setViewport( mApp->getViewportBounds() );
    
	gl::disableDepthRead();
	gl::disableDepthWrite();
	gl::enableAlphaBlending();
	gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
	
    const float starDrawMinAlpha = 0.001f;
	float power = mStarfieldAlpha;
	gl::color( Color( power, power, power ) );
	if( power < starDrawMinAlpha ){
		gl::enableAlphaBlending();
	} else {
		gl::enableAdditiveBlending();
	}
    
	gl::setMatrices( getCamera() );
    
	
	// DRAW MILKYWAY
	gl::enable( GL_TEXTURE_2D );
	if( power > 0.01f ){
		gl::pushMatrices();
		gl::translate( getCamera().getEyePoint() );
		gl::rotate( Vec3f( 75.0f, 0.0f, 0.0f ) );
		gl::color( ColorA( 1.0f, 1.0f, 1.0f, power * mScalePer ) );
		mMilkyWayTex.bind();
		gl::drawSphere( Vec3f::zero(), 195000.0f, 32 );
		gl::popMatrices();
	}
	gl::disable( GL_TEXTURE_2D );
	
	// FAINT STARS
	if( mRenderFaintStars ){
        glDisable(GL_POINT_SPRITE_ARB);
        glDisable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
        glEnable(GL_POINT_SPRITE);
        glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
        glPointSize(1.0f);
        
		if( power < starDrawMinAlpha )
			gl::enableAlphaBlending();
		else
			gl::enableAdditiveBlending();
		
		mSpectrumTex.bind( 0 );
		mFaintStarsShader.bind();
		mFaintStarsShader.uniform( "spectrumTex", 0 );
		mFaintStarsShader.uniform( "scale", mScale );
		mFaintStarsShader.uniform( "power", power );
		mFaintStarsShader.uniform( "time", (float)getElapsedSeconds() );
		mFaintStarsShader.uniform( "roomDims", Vec3f( 350.0f, 200.0f, 350.0f ) );//mRoom.getDims() );
        gl::color( 1.0f, 1.0f, 1.0f, mStarfieldAlpha );
		gl::draw( mFaintVbo );
		mFaintStarsShader.unbind();
	}
    
	gl::enable( GL_TEXTURE_2D );
	
	// DRAW STARS
	if( mRenderBrightStars ){
		if( power < starDrawMinAlpha ){
			gl::enableAlphaBlending();
			mDarkStarTex.bind( 1 );
		} else {
			gl::enableAdditiveBlending();
			mStarTex.bind( 1 );
		}
        
		mPositionFbo.bindTexture( 0 );
		mSpectrumTex.bind( 2 );
		mBrightStarsShader.bind();
		mBrightStarsShader.uniform( "position", 0 );
		mBrightStarsShader.uniform( "starTex", 1 );
		mBrightStarsShader.uniform( "spectrumTex", 2 );
		mBrightStarsShader.uniform( "scale", mScale );
		mBrightStarsShader.uniform( "power", power );
		mBrightStarsShader.uniform( "roomDims", Vec3f( 350.0f, 200.0f, 350.0f ));//mRoom.getDims() );
		mBrightStarsShader.uniform( "mvMatrix", getCamera().getProjectionMatrix() * getCamera().getModelViewMatrix() );
		mBrightStarsShader.uniform( "eyePos", getCamera().getEyePoint() );
        gl::color( mStarfieldAlpha, mStarfieldAlpha, mStarfieldAlpha, 1.0f );
		if( power > starDrawMinAlpha ){
			mBrightStarsShader.uniform( "texScale", 0.5f );
			gl::draw( mBrightVbo );
			
			// IF YOU WANT MORE GLOW AROUND MAJOR STARS, USE THESE TWO LINES
            if( mMoreGlow )
            {
                mStarGlowTex.bind( 1 );
                mBrightStarsShader.uniform( "texScale", 0.1f );
            }
			gl::draw( mBrightVbo );
		} else {
			mBrightStarsShader.uniform( "texScale", 0.5f );
			gl::draw( mBrightVbo );
		}
		mBrightStarsShader.unbind();
	}
	
	gl::disable( GL_TEXTURE_2D );
	
	gl::popMatrices();
	
	// DRAW NAMES
	if( mRenderNames ){
		gl::setMatricesWindow( mApp->getViewportSize(), true );
        
        gl::enableAlphaBlending();
        gl::color(0.0f,0.0f,0.0f, 1.0f-mStarfieldAlpha);
        gl::drawSolidRect( mApp->getViewportBounds() );
        
        if( power < starDrawMinAlpha ){
			gl::enableAlphaBlending();
		} else {
			gl::enableAdditiveBlending();
		}
        
        gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
		
		BOOST_FOREACH( Star* &s, mNamedStars ){
            if( mShowSol || s != mSol ) {
                s->drawName( mMousePos, mNamesAlpha * mScalePer, math<float>::max( sqrt( mScalePer ) - 0.1f, 0.0f ) );
            }
		}
	}
	
    /*
	if( getElapsedFrames()%60 == 59 ){
		console() << "[catalog] FPS: " << getAverageFps() << std::endl;
	}
     */
    //
    ////////------------------------------------------------------
    
    //drawHud();
    
    gl::popMatrices();
}


// ----------------------------------------------------------------
//
/*
void Catalog::drawPoints()
{
    if( mShowAllPoints )
    {
        glCallList(mDisplayListPoints);
    }
}
*/

// ----------------------------------------------------------------
//
void Catalog::drawDebug()
{
    //gl::pushMatrices();
    //gl::setMatricesWindow( mApp->getViewportWidth(), mApp->getViewportHeight() );

    //gl::popMatrices();
}

#pragma mark - Input

// ----------------------------------------------------------------
//
bool Catalog::handleKeyDown(const KeyEvent& keyEvent)
{
    bool handled = true;
    
    switch (keyEvent.getCode()) 
    {
        case KeyEvent::KEY_SPACE:
            reset();
            handled = false;
            break;
            
        default:
            handled = false;
            break;
    }
    
    return handled;
}

void Catalog::handleMouseDown( const MouseEvent& event )
{
    if( event.isRight() ){
        //		mArcball.mouseDown( event.getPos() );
		mWasRightButtonLastClicked = true;
	} else {
		mWasRightButtonLastClicked = false;
	}
	mMouseTimePressed = getElapsedSeconds();
	mMouseDownPos = event.getPos();
	mMousePressed = true;
	mMouseOffset = Vec2f::zero();
    if( mCamType == CAM_STAR )
    {
        //mStarCam.mouseDown( mMouseDownPos );
    }
}

void Catalog::handleMouseUp( const MouseEvent& event )
{
	mMouseTimeReleased	= getElapsedSeconds();
	mMousePressed = false;
	mMouseOffset = Vec2f::zero();
    if( mCamType == CAM_STAR )
    {
        //mStarCam.mouseUp( event.getPos() );
    }
}

void Catalog::handleMouseMove( const MouseEvent& event )
{
    mMousePos = event.getPos();
}

void Catalog::handleMouseDrag( const MouseEvent& event )
{
	handleMouseMove( event );
	mMouseOffset = ( mMousePos - mMouseDownPos ) * 0.4f;
    if( mCamType == CAM_STAR )
    {
        //TODO
        //mStarCam.mouseDown( (mMouseDownPos+mMouseOffset) );
    }
}

////////------------------------------------------------------
//



#pragma mark - OLD SHIT

// ----------------------------------------------------------------
//
void Catalog::setFboPositions( gl::Fbo &fbo )
{
	int numBrightStars = mBrightStars.size();
    
	int index = 0;
	
	Surface32f posSurface( fbo.getTexture() );
	Surface32f::Iter it = posSurface.getIter();
	while( it.line() ){
		while( it.pixel() ){
			Vec3f pos = Vec3f( 1000000.0f, 0.0f, 0.0f );
			float col = 0.4f;
			float rad = 0.0f;
			if( index < numBrightStars )
            {
                //if( mShowSol || mBrightStars[index] != mSol )
                {
                    pos = mBrightStars[index]->mPos;
                    col = mBrightStars[index]->mColor;
                    rad = floor( constrain( ( ( 6.0f - ( mBrightStars[index]->mAbsoluteMag ) )/6.0f ), 0.3f, 1.0f ) * 3.0f * 1000 );
                }
			}
			it.r() = pos.x;
			it.g() = pos.y;
			it.b() = pos.z;
			it.a() = rad + col;
            
			index ++;
		}
	}
    
	gl::Texture posTexture( posSurface );
	fbo.bindFramebuffer();
	gl::setMatricesWindow( fbo.getSize(), false );
	gl::setViewport( fbo.getBounds() );
	gl::clear( ColorA( 0, 0, 0, 0 ), true );
	gl::draw( posTexture );
	fbo.unbindFramebuffer();
}

// ----------------------------------------------------------------
//
void Catalog::initBrightVbo()
{
	gl::VboMesh::Layout layout;
	layout.setStaticPositions();
	layout.setStaticTexCoords2d();
	layout.setStaticColorsRGB();
	
	int numVertices = FBO_WIDTH * FBO_HEIGHT;
	// 1 quad per particle
	// 2 triangles make up the quad
	// 3 points per triangle
	mBrightVbo		= gl::VboMesh( numVertices * 2 * 3, 0, layout, GL_TRIANGLES );
	
	float s = 0.5f;
	Vec3f p0( -s, -s, 0.0f );
	Vec3f p1( -s,  s, 0.0f );
	Vec3f p2(  s,  s, 0.0f );
	Vec3f p3(  s, -s, 0.0f );
	
	Vec2f t0( 0.0f, 0.0f );
	Vec2f t1( 0.0f, 1.0f );
	Vec2f t2( 1.0f, 1.0f );
	Vec2f t3( 1.0f, 0.0f );
	
	vector<Vec3f>		positions;
	vector<Vec2f>		texCoords;
	vector<Color>		colors;
	
	for( int x = 0; x < FBO_WIDTH; ++x ) {
		for( int y = 0; y < FBO_HEIGHT; ++y ) {
			float u = (float)x/(float)FBO_WIDTH;
			float v = (float)y/(float)FBO_HEIGHT;
			Color c = Color( u, v, 0.0f );
			
			positions.push_back( p0 );
			positions.push_back( p1 );
			positions.push_back( p2 );
			texCoords.push_back( t0 );
			texCoords.push_back( t1 );
			texCoords.push_back( t2 );
			colors.push_back( c );
			colors.push_back( c );
			colors.push_back( c );
			
			positions.push_back( p0 );
			positions.push_back( p2 );
			positions.push_back( p3 );
			texCoords.push_back( t0 );
			texCoords.push_back( t2 );
			texCoords.push_back( t3 );
			colors.push_back( c );
			colors.push_back( c );
			colors.push_back( c );
		}
	}
	
	mBrightVbo.bufferPositions( positions );
	mBrightVbo.bufferTexCoords2d( 0, texCoords );
	mBrightVbo.bufferColorsRGB( colors );
	mBrightVbo.unbindBuffers();
}

// ----------------------------------------------------------------
//
void Catalog::initFaintVbo()
{
	gl::VboMesh::Layout layout;
	layout.setStaticPositions();
	layout.setStaticColorsRGB();
	
	int numFaintStars	= mFaintStars.size();
	mFaintVbo			= gl::VboMesh( numFaintStars, 0, layout, GL_POINTS );
	vector<Vec3f> positions;
	vector<Color> colors;
    
	for( int i=0; i<numFaintStars; i++ ){
		positions.push_back( mFaintStars[i]->mPos );
		colors.push_back( Color( mFaintStars[i]->mColor, 0.0f, 0.0f ) );
	}
	
	mFaintVbo.bufferPositions( positions );
	mFaintVbo.bufferColorsRGB( colors );
	mFaintVbo.unbindBuffers();
}

void Catalog::parseStarData( const fs::path &path )
{
    console() << "[catalog] loading HYG star database..." << std::endl;
	std::string line;
	std::ifstream myfile( path.c_str() );
	
	if( myfile.is_open() ){
		int i=0;
		while( !myfile.eof() ){
			std::getline( myfile, line );
            if( line[0] != '#' )
            {
                createStar( line, i );
            }
			++i;
		}
		
		myfile.close();
	} else console() << "[catalog] ERROR: unable to read data file";
    
    console() << "[catalog] Stars: " << mStars.size() << " / Bright: " << mBrightStars.size() << " / Faint: " << mFaintStars.size() << " / Named: " << mNamedStars.size() << std::endl;
}

void Catalog::createStar( const std::string &text, int lineNumber )
{
	tokenizer< escaped_list_separator<char> > tokens(text);
	int index = 0;
	double ra, dec, dist;
	float appMag, absMag;
	float colIndex;
    unsigned long hd;
    unsigned long hip;
	std::string name;
	std::string spectrum;
	//0			 1   2  3     4     5    6  7   8    9      10     11       12
	//lineNumber,hip,hd,gname,pname,name,ra,dec,dist,appMag,absMag,spectrum,colIndex;
	BOOST_FOREACH(string t, tokens)
	{
        if( t.length() == 0 )
        {
            index ++;
            continue;
        }
        
        switch( index )
        {
            case 1:
                hip = lexical_cast<unsigned long>(t);
                break;
            case 2:
                hd = lexical_cast<unsigned long>(t);
                break;
            case 5:
                if( t.length() > 1 ){
                    name = t;
                } else {
                    name = "";
                }
                break;
            case 6:
                ra = lexical_cast<double>(t);
                break;
            case 7:
                dec = lexical_cast<double>(t);
                break;
            case 8:
                dist = lexical_cast<double>(t);
                break;
            case 9:
                appMag = lexical_cast<float>(t);
                break;
            case 10:
                absMag = lexical_cast<float>(t);
                break;
            case 11:
                spectrum = t;
                break;
            case 12:
                if( t != " " ){
                    colIndex = lexical_cast<float>(t);
                } else {
                    colIndex = 0.0f;
                }
                break;
                
            default:
                break;
		}
		
		index ++;
	}
	
	Vec3f pos = convertToCartesian( ra, dec, dist );
	
	//float mag = ( 80 - appMag ) * 0.1f;
	//Color col = Color( mag, mag, mag );
	
	float color = constrain( colIndex, 0.0f, 1.0f );//0.0f;
    //	if( name.length() > 0 ){
    //		char sp = spectrum[0];
    //		
    //		switch( sp ){
    //			case 'O':	color = 0.0f;	break;
    //			case 'B':	color = 0.166f;	break;
    //			case 'A':	color = 0.2f;	break;
    //			case 'F':	color = 0.5f;	break;
    //			case 'G':	color = 0.666f;	break;
    //			case 'K':	color = 0.833f;	break;
    //			case 'M':	color = 1.0f;	break;
    //			default:					break;
    //		}
    //	}
	
	// THIS FEELS WRONG. ASK ABOUT THE RIGHT WAY TO DO THIS.
	Star *star = new Star( this, pos, appMag, absMag, color, name, spectrum, mFontBlackT, mFontBlackS );
    mStars.push_back( star );
    
    if( hip > 0 )
    {
        mStarsHIP[hip] = star;
    }
    
    if( hd > 0 )
    {
        mStarsHD[hip] = star;
    }

    if( appMag < 6.0f || name.length() > 1 ){

    if( name != "Sol" )
        mBrightStars.push_back( star );
    } else {
        mFaintStars.push_back( star );
    }
	if( name.length() > 1 ){
		mNamedStars.push_back( star );
		
        if( name == "Sol" )
        {
            mSol = star;
        }
	}
}

void Catalog::parsePlanetData( const fs::path &path )
{
    console() << "[catalog] loading Kepler planet database..." << std::endl;
	std::string line;
	std::ifstream myfile( path.c_str() );
    
    int numPlanets = 0;
	
	if( myfile.is_open() ){
		int i=0;
		while( !myfile.eof() ){
			std::getline( myfile, line );
            if( line[0] != '#' )
            {
                if( createPlanet( line, i ) )
                    numPlanets++;
            }
			++i;
		}
        
        console() << "[catalog] " << numPlanets << " / " << i << " planets added" << " forming " << mStarsWithPlanets.size() << " systems" << std::endl;
		
		myfile.close();
	} else console() << "[catalog] ERROR: unable to read data file";
}

bool Catalog::createPlanet( const std::string &text, int lineNumber )
{
	tokenizer< escaped_list_separator<char> > tokens(text);
	int index = 0;
	double orbper, orbsmax, orbeccen, mass, radius, orbincl, stmass;
	std::string hostname;
    unsigned long hdname = 0;
    unsigned long hipname = 0;
    std::string name;
    
	//0-pl_hostname,1-pl_letter,2-pl_orbper,3-pl_orbsmax,4-pl_orbeccen,5-pl_massj,6-pl_radj,7-pl_dens,8-pl_orbincl,9-pl_ttvflag,10-ra,11-dec,12-st_dist,13-st_vj,14-st_teff,15-st_mass,16-st_rad,17-hd_name,18-hip_name
	BOOST_FOREACH(string t, tokens)
	{
        switch(index)
        {
            case 0:
                if( t.length() > 1 )
                {
                    hostname = t;
                }
                else
                {
                    hostname = "";
                }
                break;
            case 1:
                if( t.length() > 0 )
                {
                    name = t;
                }
                else
                {
                    name = "";
                }
                break;
                
            case 2:
                if( t.length() > 0 )
                {
                    orbper = lexical_cast<double>(t);
                }
                else
                {
                    orbper = 0.0f;
                }
                break;
            case 3:
                if( t.length() > 0 )
                {
                    orbsmax = lexical_cast<double>(t);
                }
                else
                {
                    orbsmax = 0.0f;
                }
                break;
            case 4:
                if( t.length() > 0 )
                {
                    orbeccen = lexical_cast<double>(t);
                }
                else
                {
                    orbeccen = 0.0f;
                }
                break;
            case 5:
                if( t.length() > 0 )
                {
                    mass = lexical_cast<double>(t);
                }
                else
                {
                    mass = 0.0f;
                }
                break;
            case 6:
                if( t.length() > 0 )
                {
                    radius = lexical_cast<double>(t);
                }
                else
                {
                    radius = 0.0f;
                }
                break;
            case 8:
                if( t.length() > 0 )
                {
                    orbincl = lexical_cast<double>(t);
                }
                else
                {
                    orbincl = 0.0f;
                }
                break;
            case 15:
                if( t.length() > 0 )
                {
                    stmass = lexical_cast<double>(t);
                }
                else
                {
                    stmass = 0.0f;
                }
                break;
            case 17:
                if( t.length() > 0 )
                {
                    hdname = lexical_cast<unsigned long>(t);
                }
                else
                {
                    hdname = 0;
                }
                break;
            case 18:
                if( t.length() > 0 )
                {
                    hipname = lexical_cast<unsigned long>(t);
                }
                else
                {
                    hipname = 0;
                }
                break;
                
            default:
                break;
		}
		
		index ++;
	}
	
    Star* star = NULL;
    // star by HIP name
    
    if( hipname > 0 )
    {
        tStarMap::iterator it = mStarsHIP.find(hipname);
        if( it != mStarsHIP.end() )
        {
            star = it->second;
        }
    }

    if( star == NULL && hdname > 0 )
    {
        tStarMap::iterator it = mStarsHD.find(hdname);
        if( it != mStarsHD.end() )
        {
            star = it->second;
        }
    }
    
    if( star != NULL )
    {
        // only add planets attached to named/bright stars
        if( star->mApparentMag >= 6.0f || star->mName.length() < 1 )
        {
            return false;
        }
        
        // add the star the first time
        if( star->mPlanets.size() == 0 )
        {
            mStarsWithPlanets.push_back(star);
        }
        
        Planet *planet = new Planet( this, star, name, orbper, orbsmax, orbincl, orbeccen, mass, radius, stmass );
        star->addPlanet( planet );
        //console() << "[catalog] added planet: " << hostname << " " << name << std::endl;
        return true;
    }
    else
    {
        //console() << "[catalog] host star not found for planet: " << hostname << " " << name << std::endl;
        return false;
    }
}

Vec3f Catalog::convertToCartesian( double ra, double dec, double dist )
{
	Vec3f pos;
	float alpha = toRadians( ra * 15.0 );
	float delta = toRadians( dec );
	
	pos.x = ( dist * cos( delta ) ) * cos( alpha ); 
	pos.y = ( dist * cos( delta ) ) * sin( alpha );
	pos.z = dist * sin( delta );
	
	return pos;
}

void Catalog::selectStar( bool wasRightClick )
{
	console() << "[catalog] select star" << std::endl;
	Star *touchedStar = NULL;
	float closestDist = 100000.0f;
	BOOST_FOREACH( Star* &s, mBrightStars ){
		if( s->mDistToMouse < 40.0f ){
			if( s->mDistToMouse < closestDist ){
				closestDist = s->mDistToMouse;
				touchedStar = s;
			}
		}
	}
	
	if( touchedStar ){
		if( wasRightClick ){
			setHomeStar( touchedStar );
		} else {
			setDestStar( touchedStar );
		}
	}
}

//
////////------------------------------------------------------

void Catalog::setHomeStar( Star* target )
{
    console() << "[catalog] HOME -> " << target->mName << std::endl;
    mHomeStar = target;
}

void Catalog::setDestStar( Star* target )
{
    console() << "[catalog] DEST -> " << target->mName << std::endl;
    if( mDestStar ){
        mDestStar->mIsSelected = false;
    }
    mDestStar = target;
    mDestStar->mIsSelected = true;
    
    Orbiter* orbiterScene = static_cast<Orbiter*>(mApp->getScene("orbiter"));
    
    if( orbiterScene && orbiterScene->isRunning() )
    {
        if( target == mSol )
        {
            orbiterScene->createSystem(NULL);
        }
        else
        {
            orbiterScene->createSystem(target);
        }
    }
}

bool Catalog::setRandomHome()
{
    int index = Rand::randInt( mBrightStars.size() );
    setHomeStar( mBrightStars[index] );
    return false;
}

bool Catalog::setNextKeplerStar()
{
    mNextKeplerIndex++;
    if( mNextKeplerIndex == mStarsWithPlanets.size() )
    {
        mNextKeplerIndex = 0;
    }
    return setKeplerStar();
}

bool Catalog::setPrevKeplerStar()
{
    mNextKeplerIndex--;
    if( mNextKeplerIndex < 0 )
    {
        mNextKeplerIndex = mStarsWithPlanets.size()-1;
    }
    return setKeplerStar();
}

bool Catalog::setKeplerStar()
{
    setHomeStar( mStarsWithPlanets[mNextKeplerIndex] );
    return false;
}

bool Catalog::setRandomDest()
{
    int index = Rand::randInt( mStarsWithPlanets.size() );
    setDestStar( mStarsWithPlanets[index] );
    return false;
}

bool Catalog::setSolHome()
{
    setHomeStar( mSol );
    return false;
}

bool Catalog::setSolDest()
{
    setDestStar( mSol );
    return false;
}

const Camera& Catalog::getCamera()
{
    switch( mCamType )
    {
        case CAM_MANUAL:
            return Scene::getCamera();
            
        case CAM_SPRING:
            return mSpringCam.getCam();
            
        case CAM_STAR:
            return mStarCam.getCamera();
            
        case CAM_ORBITER:
        {
            Orbiter* orbiterScene = static_cast<Orbiter*>(mApp->getScene("orbiter"));
            
            if( orbiterScene && orbiterScene->isRunning() && orbiterScene->getCamType() != Orbiter::CAM_CATALOG )
            {
                return orbiterScene->getCamera();
            }
            else
            {
                return mSpringCam.getCam();
            }
        }
        
        default:
            return mSpringCam.getCam();
    }
}

bool Catalog::setCamType()
{
    if( mCamType == CAM_ORBITER )
    {
        //TODO: why does Sol not line up with Orbiter???
        //mBrightStars.erase(remove(mBrightStars.begin(), mBrightStars.end(), mSol), mBrightStars.end());
    }
    else
    {
        //mBrightStars.push_back(mSol);
    }
    
    return false;
}

void Catalog::updateAudioResponse()
{
    AudioInput& audioInput = mApp->getAudioInput();
	
    // Get data
    //float * freqData = audioInput.getFft()->getAmplitude();
    //float * timeData = audioInput.getFft()->getData();
    int32_t dataSize = audioInput.getFft()->getBinSize();
    const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
    int maxData = std::min( (int)mNamedStars.size(), (int)dataSize );
    
    // Iterate through data
    for (int32_t i = 0; i < mNamedStars.size(); i++)
    {
        int index = i % dataSize;
        float x = fftLogData[index].x;
        float y = fftLogData[index].y;
        
        mNamedStars[i]->mAudioPer = x * y * mLabelBrightnessByAudio;
    }
}
