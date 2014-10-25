//
//  TextOrbit.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2012-12-28.
//
//


#include "TextOrbit.h"
#include "Interface.h"
#include "OculonApp.h"
#include "cinder/Camera.h"
#include "cinder/ImageIo.h"
#include "cinder/app/AppBasic.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define TRAIL_LENGTH	16000

TextOrbit::TextOrbit()
: Scene("TextOrbit")
{
}

TextOrbit::~TextOrbit()
{
}

void TextOrbit::setup()
{
    Scene::setup();

	// load texture
//	try { mTexture = gl::Texture( loadImage( loadResource("spectrum.png") ) ); }
//	catch( const std::exception &e ) { console() << e.what() << std::endl; }

	Font customFont( "Menlo", 100 );
	gl::TextureFont::Format f;
	f.enableMipmapping( true );
	mTextureFont = gl::TextureFont::create( customFont, f );
    
    addChar('O');
    addChar('R');
    addChar('B');
    addChar('I');
    addChar('T');
    addChar('A');
    addChar('L');
}

void TextOrbit::setupInterface()
{
}

//void TextOrbit::setupDebugInterface()
//{
//    Scene::setupDebugInterface(); // add all interface params
//}


void TextOrbit::addChar( char c )
{
	c = tolower( c ); // Alphabet-IV.tff seems to be missing some capital letters (strange, since it's an all-caps font)
	int count = mCharacters.size();
	
//	if( count > 0 )
//		mSceneDestMatrix.translate( Vec3f( mCharacters.back().getKernBounds().getWidth() / 2.0f, 0.0f, 0.0f ) );
//	
//	Matrix44f randStartMatrix = mSceneDestMatrix;
//	randStartMatrix.translate( getRandomVec3f( 100.0f, 600.0f ) );
//	randStartMatrix.rotate( getRandomVec3f( 2.0f, 6.0f ) );
    
    Matrix44f startMatrix = Matrix44f::identity();
    startMatrix.translate( Vec3f( (mApp->getViewportHeight() / 2.0f) + 30 * count, mApp->getViewportHeight() / 2.0f, 0.0f ) );
	
	mCharacters.push_back( Character( mTextureFont, string( &c, 1 ), startMatrix ) );
	
//	mSceneDestMatrix.translate( Vec3f( mCharacters.back().getKernBounds().getWidth() / 2.0f, 0.0f, 0.0f ) );
//    
//	float t = (count + 281)/50.0f;
//	mSceneDestMatrix.rotate( Vec3f( sin(t)*0.1f, cos(t)*0.2f, cos(t)*0.05f ) ); // makes the scene meander
//	
//	mCharacters.back().animIn( timeline(), mSceneDestMatrix );
//	
//	timeline().apply( &mSceneMatrix, mSceneDestMatrix, 1.0f, EaseOutAtan( 10 ) );
}

void TextOrbit::update(double dt)
{
    for( vector<Character>::iterator it = mCharacters.begin(); it != mCharacters.end(); ++it )
    {
        it->update(dt);
    }
}

void TextOrbit::draw()
{
	gl::enableAlphaBlending();

	//gl::enableDepthRead();
	//gl::enableDepthWrite();
    gl::disableDepthWrite();
	gl::disableDepthRead();
    
    
    //gl::enable( GL_TEXTURE_2D );
    //gl::disable( GL_TEXTURE_2D );
	gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
    
    //CameraPersp cam( getWindowWidth(), getWindowHeight(), 60.0f, 0.1f, 500.0f );
    //cam.setEyePoint( Vec3f(0, 0, -100.0f) );
    //cam.setCenterOfInterestPoint( Vec3f::zero() );

	// enable 3D camera
	gl::pushMatrices();
	//gl::setMatrices( getCamera() );
    //gl::setMatricesWindow( mApp->getViewportWidth(), mApp->getViewportHeight() );
    CameraOrtho textCam(0.0f, mApp->getViewportWidth(), 0.0f, mApp->getViewportHeight(), 0.0f, 50.f);
    gl::setMatrices(textCam);

    for( vector<Character>::iterator it = mCharacters.begin(); it != mCharacters.end(); ++it )
    {
		it->draw();
    }
    
	// restore camera and render states
    gl::disable( GL_TEXTURE_2D );
	gl::popMatrices();

	gl::disableDepthWrite();
	gl::disableDepthRead();

	gl::disableAlphaBlending();
}
