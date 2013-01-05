/*
 *  FisheyeTest.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "OculonApp.h"
#include "FisheyeTest.h"
#include "cinder/Rand.h"
#include "cinder/Utilities.h"
#include "Resources.h"


using namespace ci;

FisheyeTest::FisheyeTest()
: Scene("fisheye")
{
}

FisheyeTest::~FisheyeTest()
{
}

void FisheyeTest::setup()
{
    Scene::setup();
    
    mUseShader = false;
    
    try 
    {
		mShader = gl::GlslProg( loadResource( RES_FISHEYE_VERT ) );
	}
	catch( gl::GlslProgCompileExc &exc ) 
    {
		std::cout << "Shader compile error: " << std::endl;
		std::cout << exc.what();
	}
	catch( ... ) 
    {
		std::cout << "Unable to load shader" << std::endl;
	}
    
    mTexture = gl::Texture( loadImage( loadResource( RES_ORBITER_JUPITER ) ) );
}

void FisheyeTest::update(double dt)
{
}

void FisheyeTest::draw()
{
    gl::pushMatrices();
    
    //CameraOrtho cam(0.0f, app::getWindowWidth(), app::getWindowHeight(), 0.0f, 0.0f, 1.0f);
    //CameraPersp cam(app::getWindowWidth(), app::getWindowHeight(), 60.0f);
    //gl::setMatrices(cam);
    //gl::setMatricesWindowPersp(app::getWindowWidth(), app::getWindowHeight());    
    gl::enableDepthRead(false);
    //gl::enableDepthWrite(false);
    
    
    if( mUseShader )
    {
        mShader.bind();
    }
    
    gl::draw( mTexture );
    
    const float width = mApp->getWindowWidth();
    const float height = mApp->getWindowHeight();
    
    const int dist = 10;
    
    gl::color( 0.75f, 0.75f, 0.75f );
    for(int i = 0; i < width; i+=dist )
    {
        gl::drawLine( Vec2f( i, 0.0f ), Vec2f( i, height ) );
    }
                   
    for(int i = 0; i < height; i+=dist )
    {
        gl::drawLine( Vec2f( 0.0f, i ), Vec2f( width, i ) );
    }
    
    for( int i = 0; i < 5; ++i )
    {
        gl::color( Rand::randFloat(), Rand::randFloat(), Rand::randFloat() );
        gl::drawSolidRect( Rectf( Rand::randFloat(width-100), Rand::randFloat(height-100), Rand::randFloat(width), Rand::randFloat(height) ) );
    }
    
    if( mUseShader )
    {
        mShader.unbind();
    }
    
    gl::enableAlphaBlending();
    gl::enableDepthRead(true);
    //gl::enableDepthWrite(true);
    
    gl::popMatrices();
}

bool FisheyeTest::handleKeyDown(const KeyEvent& keyEvent)
{
    bool handled = true;
    
    switch (keyEvent.getChar()) 
    {
            /*
        case 'o':
            mMoviePlayer1.loadMoviePrompt();
            break;
             */
        case 'e':
            mUseShader = !mUseShader;
            break;
        default:
            handled = false;
            break;
    }
    
    return handled;    
}
