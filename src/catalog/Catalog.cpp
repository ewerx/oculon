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

#include "Orbiter.h"

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

using namespace ci;
using namespace ci::app;
using namespace boost;

const int Catalog::FBO_WIDTH = 78;
const int Catalog::FBO_HEIGHT = 78;

// ----------------------------------------------------------------
//
Catalog::Catalog()
: Scene("catalog")
{
    for( int i=0; i < TB_COUNT; ++i )
    {
        mTextBox[i] = new TextEntity(this);
        assert(mTextBox[i] != NULL && "out of memory, how ridiculous");
    }
}

// ----------------------------------------------------------------
//
Catalog::~Catalog()
{
}

// ----------------------------------------------------------------
//
void Catalog::setup()
{
    // params
    mShowLabels = true;
    mCamType = CAM_SPRING;
    
    // assets
    
    ////////------------------------------------------------------
    //
    // CAMERA	
	mSpringCam		= SpringCam( -350.0f, getWindowAspectRatio() );
    
	// LOAD SHADERS
	try {
		mBrightStarsShader	= gl::GlslProg( loadResource( RES_BRIGHT_STARS_VERT ), loadResource( RES_BRIGHT_STARS_FRAG ) );
		mFaintStarsShader	= gl::GlslProg( loadResource( RES_FAINT_STARS_VERT ), loadResource( RES_FAINT_STARS_FRAG ) );
		//mRoomShader			= gl::GlslProg( loadResource( "../resources/shaders/room.vert" ), loadResource( "../resources/shaders/room.frag" ) );
	} catch( gl::GlslProgCompileExc e ) {
		std::cout << e.what() << std::endl;
		//quit();
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
	
	// ROOM
	//gl::Fbo::Format roomFormat;
	//roomFormat.setColorInternalFormat( GL_RGB );
	//mRoomFbo			= gl::Fbo( APP_WIDTH/ROOM_FBO_RES, APP_HEIGHT/ROOM_FBO_RES, roomFormat );
	//bool isPowerOn		= false;
	//bool isGravityOn	= true;
	//mRoom				= Room( Vec3f( 350.0f, 200.0f, 350.0f ), isPowerOn, isGravityOn );	
	//mRoom.init();
	
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
	parseData( App::getResourcePath( "starData.txt" ) );
	mTotalTouringStars = mTouringStars.size();
	mHomeStar	= NULL;
	mDestStar	= NULL;
	
	std::cout << mStars.size() << std::endl;
    
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
        
    initStars();
    
    reset();
}

// ----------------------------------------------------------------
//
void Catalog::initStars()
{
    // points display list
    /*
    mDisplayListPoints = glGenLists(1);
    glNewList(mDisplayListPoints, GL_COMPILE);
    glBegin(GL_POINTS);
    for(QuakeList::iterator it = mQuakes.begin(); 
        it != mQuakes.end();
        ++it)
    {
        const Vec3f& pos = (*it)->getPosition();
        glVertex2f(pos.x, pos.y);
    }
    glEnd();
    glEndList();
     */
}

// ----------------------------------------------------------------
//
void Catalog::setupInterface()
{
    
    mInterface->addEnum(CreateEnumParam( "Cam Type", (int*)(&mCamType) )
                        .maxValue(CAM_COUNT)
                        .oscReceiver(getName(), "camtype")
                        .isVertical());
    mInterface->addParam(CreateBoolParam( "show names", &mRenderNames )
                        .oscReceiver(getName(), "shownames"));
    mInterface->addParam(CreateBoolParam( "show faint", &mRenderFaintStars )
                         .oscReceiver(getName(), "showfaint"));
    mInterface->addParam(CreateBoolParam( "show bright", &mRenderBrightStars )
                         .oscReceiver(getName(), "showbright"));
    /*
    mInterface->addParam(CreateFloatParam("BPM", &mBpm)
                         .minValue(60.0f)
                         .maxValue(150.0f)
                         .oscReceiver(mName,"bpm")
                         .sendFeedback());    
    mInterface->addButton(CreateTriggerParam("Trigger Quake", NULL)
                          .oscReceiver(mName,"quaketrigger"))->registerCallback( this, &Catalog::triggerNextQuake );
    */
}

// ----------------------------------------------------------------
//
void Catalog::setupDebugInterface()
{
    mDebugParams.addParam("Show Labels", &mShowLabels );
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
    for( int i = 0; i < TB_COUNT; ++i )
    {
        mTextBox[i]->resize();
    }
}

// ----------------------------------------------------------------
//
void Catalog::update(double dt)
{
    ////////------------------------------------------------------
    //
    if( getElapsedSeconds() - mMouseTimePressed < mMouseTimeThresh && !mMousePressed ){
		mMouseTimePressed = 0.0f;
		selectStar( mWasRightButtonLastClicked );
	}
	
	//mRoom.update();
	
	mScaleDest -= ( mScaleDest - mMaxScale ) * 0.05f;
	mScale -= ( mScale - mScaleDest ) * 0.02f;
	mScalePer = mScale/mMaxScale;
	
	// CAMERA
	if( mHomeStar != NULL ){
		mSpringCam.setEye( mHomeStar->mPos + Vec3f( 100.0f, 0.0f, 40.0f ) );
	}
	
	if( mDestStar != NULL ){
		mSpringCam.setCenter( mDestStar->mPos );
	}
    
	if( mMousePressed ) 
		mSpringCam.dragCam( ( mMouseOffset ) * 0.01f, ( mMouseOffset ).length() * 0.01 );
	mSpringCam.update( 0.25f );
	
	BOOST_FOREACH( Star* &s, mBrightStars ){
		s->update( getCam(), mScale );
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
    //
    //gl::clear( Color( 0, 0, 0 ) );
	gl::color( ColorA( 1, 1, 1, 1 ) );
    
	gl::setMatricesWindow( getWindowSize(), false );
	gl::setViewport( getWindowBounds() );
    
	gl::disableDepthRead();
	gl::disableDepthWrite();
	gl::enableAlphaBlending();
	gl::enable( GL_TEXTURE_2D );
	gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
	
    /*
	// DRAW ROOM
	mRoomFbo.bindTexture();
	gl::drawSolidRect( getWindowBounds() );
	*/
	gl::setMatricesWindow( getWindowSize(), true );
	
	float power = 1.0f;//mRoom.getPower();
	gl::color( Color( power, power, power ) );
	if( power < 0.5f ){
		gl::enableAlphaBlending();
	} else {
		gl::enableAdditiveBlending();
	}
    
	gl::setMatrices( getCam() );
	
    /*
	// DRAW PANEL
	if( power < 0.1f )
		drawInfoPanel();
	*/
    
	gl::enable( GL_TEXTURE_2D );
	
	gl::pushMatrices();
    //	gl::multModelView( mArcball.getQuat() );
	
	// DRAW MILKYWAY
	if( power > 0.01f ){
		gl::pushMatrices();
		gl::translate( getCam().getEyePoint() );
		gl::rotate( Vec3f( 75.0f, 0.0f, 0.0f ) );
		gl::color( ColorA( 1.0f, 1.0f, 1.0f, power * mScalePer ) );
		mMilkyWayTex.bind();
		gl::drawSphere( Vec3f::zero(), 195000.0f, 32 );
		gl::popMatrices();
	}
	
	gl::disable( GL_TEXTURE_2D );
	
	// FAINT STARS
	if( mRenderFaintStars ){
		if( power < 0.5f )
			gl::enableAlphaBlending();
		else
			gl::enableAdditiveBlending();
		
		mSpectrumTex.bind( 0 );
		mFaintStarsShader.bind();
		mFaintStarsShader.uniform( "spectrumTex", 0 );
		mFaintStarsShader.uniform( "scale", mScale );
		mFaintStarsShader.uniform( "power", power );//mRoom.getPower() );
		mFaintStarsShader.uniform( "time", (float)getElapsedSeconds() );
		mFaintStarsShader.uniform( "roomDims", Vec3f( 350.0f, 200.0f, 350.0f ) );//mRoom.getDims() );
		gl::draw( mFaintVbo );
		mFaintStarsShader.unbind();
	}
    
	gl::enable( GL_TEXTURE_2D );
	
	// DRAW STARS
	if( mRenderBrightStars ){
		if( power < 0.5f ){
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
		mBrightStarsShader.uniform( "power", 1.0f );//mRoom.getPower() );
		mBrightStarsShader.uniform( "roomDims", 1.0f);//mRoom.getDims() );
		mBrightStarsShader.uniform( "mvMatrix", getCam().getModelViewMatrix() );
		mBrightStarsShader.uniform( "eyePos", getCam().getEyePoint() );
		if( power > 0.5f ){
			mBrightStarsShader.uniform( "texScale", 0.5f );
			gl::draw( mBrightVbo );
			
			// IF YOU WANT MORE GLOW AROUND MAJOR STARS, USE THESE TWO LINES
            //			mStarGlowTex.bind( 1 );
            //			mBrightStarsShader.uniform( "texScale", 0.1f );
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
		if( power < 0.5f ){
			gl::enableAlphaBlending();
		} else {
			gl::enableAdditiveBlending();
		}
		gl::setMatricesWindow( getWindowSize(), true );
		
		BOOST_FOREACH( Star* &s, mNamedStars ){
			s->drawName( mMousePos, power * mScalePer, math<float>::max( sqrt( mScalePer ) - 0.1f, 0.0f ) );
		}
	}
	
    /*
	if( getElapsedFrames()%60 == 59 ){
		console() << "[catalog] FPS: " << getAverageFps() << std::endl;
	}
     */
    //
    ////////------------------------------------------------------
    
    drawHud();
    
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
void Catalog::drawHud()
{
    gl::pushMatrices();
    
    const float width = mApp->getViewportWidth();
    const float height = mApp->getViewportHeight();
    
    //CameraOrtho textCam(0.0f, width, height, 0.0f, 0.0f, 10.f);
    //gl::setMatrices(textCam);
    
    if( mShowLabels )
    {
    }
    
    gl::popMatrices();
}

// ----------------------------------------------------------------
//
void Catalog::drawDebug()
{
    //gl::pushMatrices();
    //gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );

    //gl::popMatrices();
}

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
}

void Catalog::handleMouseUp( const MouseEvent& event )
{
	mMouseTimeReleased	= getElapsedSeconds();
	mMousePressed = false;
	mMouseOffset = Vec2f::zero();
}

void Catalog::handleMouseMove( const MouseEvent& event )
{
    mMousePos = event.getPos();
}

void Catalog::handleMouseDrag( const MouseEvent& event )
{
	handleMouseMove( event );
	mMouseOffset = ( mMousePos - mMouseDownPos ) * 0.4f;
}

////////------------------------------------------------------
//

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
			if( index < numBrightStars ){
				pos = mBrightStars[index]->mPos;
				col = mBrightStars[index]->mColor;
				rad = floor( constrain( ( ( 6.0f - ( mBrightStars[index]->mAbsoluteMag ) )/6.0f ), 0.3f, 1.0f ) * 3.0f * 1000 );
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

void Catalog::parseData( const fs::path &path )
{
	std::string line;
	std::ifstream myfile( path.c_str() );
	
	if( myfile.is_open() ){
		int i=0;
		while( !myfile.eof() ){
			std::getline( myfile, line );
			createStar( line, i );
			++i;
		}
		
		myfile.close();
	} else console() << "[catalog] ERROR: unable to read data file";
}

void Catalog::createStar( const std::string &text, int lineNumber )
{
	char_separator<char> sep(",");
	tokenizer< char_separator<char> > tokens(text, sep);
	int index = 0;
	double ra, dec, dist;
	float appMag, absMag;
	float colIndex;
	std::string name;
	std::string spectrum;
	//0			 1    2  3   4    5      6      7        8
	//lineNumber,name,ra,dec,dist,appMag,absMag,spectrum,colIndex;
	BOOST_FOREACH(string t, tokens)
	{
		if( index == 1 ){
			if( t.length() > 1 ){
				name = t;
			} else {
				name = "";
			}
		} else if( index == 2 ){
			ra = lexical_cast<double>(t);
			
		} else if( index == 3 ){
			dec = lexical_cast<double>(t);
			
		} else if( index == 4 ){
			dist = lexical_cast<double>(t);
			
		} else if( index == 5 ){
			appMag = lexical_cast<float>(t);
			
		} else if( index == 6 ){
			absMag = lexical_cast<float>(t);
			
		} else if( index == 7 ){
			spectrum = t;
			
		} else if( index == 8 ){
			if( t != " " ){
				colIndex = lexical_cast<float>(t);
			} else {
				colIndex = 0.0f;
			}
		}
		
		index ++;
	}
	
	Vec3f pos = convertToCartesian( ra, dec, dist );
	
	float mag = ( 80 - appMag ) * 0.1f;
	Color col = Color( mag, mag, mag );
	
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
	Star *star = new Star( pos, appMag, absMag, color, name, spectrum, mFontBlackT, mFontBlackS );
	mStars.push_back( *star );
	
	if( appMag < 6.0f || name.length() > 1 ){
		mBrightStars.push_back( star );
	} else {
		mFaintStars.push_back( star );
	}
	
	if( name.length() > 1 ){
		mNamedStars.push_back( star );
		
		if( name == "Sol" || name == "Sirius" || name == "Vega" || name == "Gliese 581" ){
			mTouringStars.push_back( star );
			console() << "[catalog] ADDED TOURING STAR: " << star->mName << " " << star->mPos << std::endl;
		}
	}
}

Vec3f Catalog::convertToCartesian( double ra, double dec, double dist )
{
	Vec3f pos;
	float RA = toRadians( ra * 15.0 );
	float DEC = toRadians( dec );
	
	pos.x = ( dist * cos( DEC ) ) * cos( RA ); 
	pos.y = ( dist * cos( DEC ) ) * sin( RA );
	pos.z = dist * sin( DEC );
	
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
			mHomeStar = touchedStar;
		} else {
			console() << "[catalog] TOUCHED " << touchedStar->mName << std::endl;
			if( mDestStar ){
				mDestStar->mIsSelected = false;
			}
			mDestStar = touchedStar;
			mDestStar->mIsSelected = true;
		}
	}
}

//
////////------------------------------------------------------

const Camera& Catalog::getCam()
{
    switch( mCamType )
    {
        case CAM_SPRING:
            return mSpringCam.getCam();
            
        case CAM_ORBITER:
        {
            Orbiter* orbiterScene = static_cast<Orbiter*>(mApp->getScene(0));
            
            if( orbiterScene && orbiterScene->isRunning() )
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
