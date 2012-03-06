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
#include "cinder/Rand.h"
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
    
    for( int i = 0; i < FBO_COUNT; ++i )
    {
        mFbo[i] = gl::Fbo( fboWidth, fboHeight, format );
    }
        
    // blur shader
    try 
    {
		//mShader = gl::GlslProg( loadResource( RES_BLUR2_VERT ), loadResource( RES_BLUR2_FRAG ) );
        mShader = gl::GlslProg( loadResource( RES_PASSTHRU_VERT ), loadResource( RES_BLUR_FRAG ) );
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
    
    try 
    {
		mTexture = gl::Texture( loadImage( loadResource( RES_ORBITER_JUPITER ) ) );
	}
	catch( ... ) 
    {
		std::cout << "unable to load the texture file!" << std::endl;
	}
    
    mVel = Vec2f(100.0f,0.0f);
    mPos = Vec2f(0.0f,mApp->getWindowHeight()/2.0f);
    
    mEnableShader = false;
    mBlurAmount = 0.5f;
    
    mFboPing = 0;
    mFboPong = 1;
}

void ShaderTest::update(double dt)
{
    /*
    gl::setMatricesWindow(mFbo[mFboPing].getSize(), false);
    gl::setViewport(mFbo[mFboPing].getBounds());
    
    glEnable(GL_TEXTURE_2D);
    
    // Loop through iteration count
    for (int i = 0; i < FBO_ITERATIONS; i++)
    {
        
        // Swap FBO indexes
        mFboPing = (mFboPing + 1) % 2;
        mFboPong = (mFboPong + 1) % 2;
        
        // Bind the "ping" FBO so we can draw onto it
        mFbo[mFboPing].bindFramebuffer();
        
        // Bind the "pong" FBO as a texture to 
        // send to the shader
        mFbo[mFboPong].bindTexture();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        
        /****** Begin process shader configuration ******/
        
        /*
         // Bind the process shader
         shaderProcess.bind();
         
         // This was the "ping" FBO we drew onto 
         // in the last iteration
         shaderProcess.uniform("texture", 0); 
         
         // This controls the definition of edges in 
         // the shader (higher is softer)
         shaderProcess.uniform("dampen", 350.0f);
         
         // These are used in a mutant variant of the 
         // Gray-Scott reaction-diffusion equation
         shaderProcess.uniform("ru", 0.33f); 
         shaderProcess.uniform("rv", 0.1f);
         shaderProcess.uniform("k", 0.06f);
         shaderProcess.uniform("f", 0.25f);
         
         // We pass in the width and height to convert normalized 
         // coordinates to screen coordinates
         shaderProcess.uniform("width", (float)FBO_WIDTH);
         shaderProcess.uniform("height", (float)FBO_HEIGHT);
         
         /****** End process shader configuration ******/
        /*
        if(mEnableShader) 
        {
            mShader.bind();
            float invWidth = 1.0f/mFbo[mFboPing].getWidth();
            float invHeight = 1.0f/mFbo[mFboPing].getHeight();
            float i = mBlurAmount;
            mShader.uniform("tex0", 0);
			mShader.uniform("blurCenterWeight", 0.5f);
            mShader.uniform("amountX", 1 * invWidth * i);
            mShader.uniform("amountY", 1 * invHeight * i);
            
        }
        
        
        // Draw the shader output onto the "ping" FBO
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        gl::drawSolidRect(mFbo[mFboPing].getBounds());
        
        // Stop the shader
        //shaderProcess.unbind();
        if(mEnableShader) 
        {
            mShader.unbind();
        }
        
        // Draw a red circle randomly on the screen
        //random.randomize();
        RectMapping windowToFBO(getWindowBounds(), mFbo[mFboPing].getBounds());
        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
        gl::drawSolidCircle(windowToFBO.map(Vec2f(
                                                  Rand::randFloat(mApp->getWindowWidth()), 
                                                  Rand::randFloat(mApp->getWindowHeight()))), 
                                            Rand::randFloat(1.0f, 30.0f), 64);
        
        // TO DO: Draw anything you want here in red. 
        //            It will refract the image texture.
        
        // Unbind the FBO to stop drawing on it
        mFbo[mFboPing].unbindFramebuffer();
        
    }
    
    */
    mPos += mVel * dt;
    
    if( mPos.x > (mApp->getWindowWidth()-100) || mPos.x < 0.0f )
    {
        mVel.x *= -1.0f;
    }
         
}

void ShaderTest::draw()
{
    gl::pushMatrices();

    Area viewport = gl::getViewport();
    
    // Clear screen and set up viewport
	gl::clear(ColorA(0.0f, 0.0f, 0.0f, 0.0f));
	gl::setMatricesWindow(getWindowSize());
	gl::setViewport(getWindowBounds());
    
    
    static float mAngle = 0.0f;
    mAngle += 0.05f;
    
    mTexture.enableAndBind();
    
    mShader.bind();
	mShader.uniform( "tex0", 0 );
	mShader.uniform( "sampleOffset", Vec2f( cos( mAngle ), sin( mAngle ) ) * ( 3.0f / getWindowWidth() ) );
    
    glColor4f(1.0f,1.0f,0.0f,1.0f);
    const float radius = 100.f;
    gl::drawSolidRect( Rectf(mPos.x, mPos.y, mPos.x + radius, mPos.y + radius) );
    
    mTexture.unbind();
    /*
	// We're in input-only mode
	if (!mEnableShader)
	{
        
		// Render the raw input
		gl::draw(mFbo[mFboPing].getTexture());
        
	}
	else
	{
		// Bind the FBO we last rendered as a texture
		mFbo[mFboPing].bindTexture();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
		// Start and configure the render shader
		//shaderRender.bind();
        
		// This is the image we've been drawing
		//shaderRender.uniform("texture", 0);
        
		// This is the texture we'll be refracting
		//shaderRender.uniform("srcTexture", 1);
        
		// Pass 
		//shaderRender.uniform("width", (float)mApp->getWindowWidth());
		//shaderRender.uniform("height", (float)mApp->getWindowHeight());
        
		// Draw rectangle
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		gl::drawSolidRect(mFbo[mFboPing].getBounds());
        
		// Stop shader
		//shaderRender.unbind();
        
	}
    
    /*
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
     */
    
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
