/*
 *  ShaderTest.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "OculonApp.h"//TODO: fix this dependency
#include "ShaderTest.h"
#include "Resources.h"
#include "cinder/Utilities.h"
#include <boost/format.hpp>


using namespace ci;

ShaderTest::ShaderTest()
{
}

ShaderTest::~ShaderTest()
{
}

void ShaderTest::setup()
{
    gl::Fbo::Format format;
    //format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
    const int fboWidth = mApp->getWindowWidth();
    const int fboHeight = mApp->getWindowHeight();
    mFboA = gl::Fbo( fboWidth, fboHeight, format );
    mFboB = gl::Fbo( fboWidth, fboHeight, format );
    mFboC = gl::Fbo( fboWidth, fboHeight, format );
        
    // blur shader
    try 
    {
		mShader = gl::GlslProg( loadResource( RES_BLUR2_VERT ), loadResource( RES_BLUR2_FRAG ) );
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
    
    mVel = Vec2f(1.0f,0.0f);
    mPos = Vec2f(0.0f,mApp->getWindowHeight()/2.0f);
    
    mEnableShader = false;
    mBlurAmount = 0.5f;
}

void ShaderTest::update(double dt)
{
    mPos += mVel;
    
    if( mPos.x > (mApp->getWindowWidth()-100) )
    {
        mVel.x *= -1.0f;
    }
}

void ShaderTest::draw()
{
    gl::pushMatrices();

    Area viewport = gl::getViewport();
    
    static bool first = true;
    
    if( first )
    {
    mFboC.bindFramebuffer();			// draw new frame on a clear transparent background
    {
        gl::setMatricesWindow( mFboA.getSize(), false );
        gl::setViewport( mFboA.getBounds() );
        gl::clear( ColorA(0,0,0,0.0f) );
        
        //glColor4f(1.0f,1.0f,0.0f,1.0f);
        //const float radius = 100.f;
        //gl::drawSolidRect( Rectf(mPos.x, mPos.y, mPos.x + radius, mPos.y + radius) );
    }
    mFboC.unbindFramebuffer();
    
    first = false;
    }
    
	mFboA.bindFramebuffer();			// draw new frame on a clear transparent background
    {
        gl::setMatricesWindow( mFboA.getSize(), false );
        gl::setViewport( mFboA.getBounds() );
        gl::clear( ColorA(0,0,0,0.0f) );
        
        glColor4f(1.0f,1.0f,0.0f,1.0f);
        const float radius = 100.f;
        gl::drawSolidRect( Rectf(mPos.x, mPos.y, mPos.x + radius, mPos.y + radius) );
    }
    mFboA.unbindFramebuffer();
	
    mFboB.bindFramebuffer();			// composite prev blurred version with new frame on clear transparent background
    {
        gl::setMatricesWindow( mFboB.getSize(), false );
        gl::setViewport( mFboB.getBounds() );
        gl::clear( ColorA(0,0,0,1.0f) );
        gl::enableAdditiveBlending();
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_ONE, GL_ONE);
        glEnable(GL_TEXTURE_2D);
        
		float c = 1.0f - 0.01f;
        glColor4f(c, c, c, 1.0f);
        
        //        gl::clear( Color::black() );
        //mFboC.bindTexture(0); // use rendered scene as texture
        //        gl::pushMatrices();
        //        gl::setMatricesWindow(512, 512, false);
        //gl::drawSolidRect( FboB.getBounds() );
        //        gl::popMatrices();
        //mFboC.unbindTexture();
        //        mFboTemporary.unbindFramebuffer();
        
        //gl::draw( mFboC.getTexture(0), mFboB.getBounds() );
		
        glColor3f(1, 1, 1);
        gl::draw( mFboA.getTexture(0), mFboB.getBounds() );
        //mFboA.bindTexture(0); // use rendered scene as texture
        //gl::drawSolidRect( FboB.getBounds() );
        //mFboA.unbindTexture();
        
        gl::draw( mFboC.getTexture(0), mFboB.getBounds() );
        
    }
    mFboB.unbindFramebuffer();
	
	
    mFboC.bindFramebuffer();
    {
        gl::setMatricesWindow( mFboC.getSize(), false );
        gl::setViewport( mFboC.getBounds() );
		gl::clear( ColorA(0,0,0,1.0f) );
        
        if(mEnableShader) 
        {
            mShader.bind();
            float invWidth = 1.0f/mFboC.getWidth();
            float invHeight = 1.0f/mFboC.getHeight();
            float i = mBlurAmount;
            mShader.uniform("amountX", 1 * invWidth * i);
            mShader.uniform("amountY", 1 * invHeight * i);
        }
		
        gl::draw( mFboB.getTexture(0), mFboC.getBounds() );
        //FboB.bindTexture(0); // use rendered scene as texture
        //gl::drawSolidRect( mFboC.getBounds() );
        //FboB.unbindTexture();
        
        if(mEnableShader) 
            mShader.unbind();
    }
    mFboC.unbindFramebuffer();
    
    
    gl::setMatricesWindow( mApp->getWindowSize() );
    glEnable(GL_TEXTURE_2D);
    glColor3f( 1, 1, 1 );
    gl::enableAdditiveBlending();
    gl::setMatricesWindow( mApp->getWindowSize() );
    gl::draw( mFboB.getTexture(), getWindowBounds() );
    
    gl::setViewport( viewport );
    
    gl::popMatrices();
}

bool ShaderTest::handleKeyDown(const KeyEvent& keyEvent)
{
    bool handled = true;
    
    switch (keyEvent.getChar()) 
    {
        case 'z':
            mEnableShader = !mEnableShader;
            break;
        default:
            handled = false;
            break;
    }
    
    return handled;    
}
