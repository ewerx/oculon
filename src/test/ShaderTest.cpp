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
#include "Interface.h"

using namespace ci;

ShaderTest::ShaderTest()
: Scene("shadertest")
{
}

ShaderTest::~ShaderTest()
{
}

void ShaderTest::setup()
{
    Scene::setup();
    
    Vec2f viewportSize( mApp->getViewportWidth(), mApp->getViewportHeight() );
    mMotionBlurRenderer.setup( viewportSize, boost::bind( &ShaderTest::drawScene, this ) );
    
    mUseFbo = false;
    // setup FBO
    gl::Fbo::Format format;
    //format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
    const int fboWidth = mApp->getViewportWidth();
    const int fboHeight = mApp->getViewportHeight();
    //format.enableMipmapping(false);
    format.enableDepthBuffer(false);
	format.setCoverageSamples(8);
	format.setSamples(4);
    
    mFboIndex = 0;
    
    for( int i = 0; i < FBO_COUNT; ++i )
    {
        mFbo[i] = gl::Fbo( fboWidth, fboHeight, format );
    }
        
    // blur shader
    try 
    {
		//mShader = gl::GlslProg( loadResource( RES_BLUR2_VERT ), loadResource( RES_BLUR2_FRAG ) );
        //mShader = gl::GlslProg( loadResource( RES_PASSTHRU_VERT ), loadResource( RES_BLUR_FRAG ) );
        mShader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_MOTIONBLUR_FRAG ) );
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
    
    /*
    try 
    {
		mTexture = gl::Texture( loadImage( loadResource( RES_ORBITER_JUPITER ) ) );
	}
	catch( ... ) 
    {
		std::cout << "unable to load the texture file!" << std::endl;
	}
    
    mTexture.bind(1);
    */
    
    mVel = Vec2f(100.0f,0.0f);
    mPos = Vec2f(0.0f,mApp->getViewportHeight()/2.0f);
    mColor = ColorA(1.0f,0.0f,0.0f,1.0f);
    
    //mEnableShader = false;
    mBlurAmount = 8.0f / mApp->getViewportWidth();
    
    
    // OSC TEST
    mApp->getOscServer().registerCallback( "/multi/1", this, &ShaderTest::handleOscMessage );
    
}

void ShaderTest::setupInterface()
{
    mInterface->addParam(CreateIntParam( "Radius", &mRadius )
                         .minValue(1)
                         .maxValue(300)
                         .oscReceiver("/1/fader2")
                         .oscSender("/1/fader2"));
    
    mInterface->addParam(CreateBoolParam( "Shader", &mUseFbo )
                         .oscReceiver("/1/toggle2")
                         .oscSender("/1/toggle2"));
    
    
    //mInterface->gui()->addParam("pos", mPos.ptr(), Vec2f::zero(), Vec2f(mApp->getViewportWidth(),mApp->getViewportHeight()));
    
    mInterface->addParam(CreateVec2fParam("pos", &mPos, Vec2f::zero(), Vec2f(mApp->getViewportWidth(),mApp->getViewportHeight()))
                         .oscReceiver(mName)
                         .isXYPad());
    //mInterface->gui()->addParam("vel", mVel.ptr(), Vec2f::zero());
    
    mInterface->addParam(CreateColorParam("color", &mColor, kMinColor, kMaxColor)
                         .oscReceiver(mName));

    mRadius = 100.0f;
}

void ShaderTest::update(double dt)
{
    //mPos += mVel * dt;
    
    if( mPos.x > (mApp->getViewportWidth()-100) || mPos.x < 0.0f )
    {
        mVel.x *= -1.0f;
    }
    
    Scene::update(dt);
}

void ShaderTest::updateBlur()
{
    
    
    /*
    mFboScene.bindFramebuffer();
    {
        glDisable( GL_TEXTURE_2D );
        //gl::clear(Color::black());
        gl::clear( ColorA(0.0f,0.0f,0.0f,0.0f) );
        
    }
	mFboScene.unbindFramebuffer();
    
    /*
    if (mEnableShader)
	{
        mShader.bind();
        mShader.uniform("tex0", 0);
        mShader.uniform("sampleOffset", Vec2f(mBlurAmount, 0.0f));
	}
    *
    
	mFbo[mFboPing].bindFramebuffer();
    {
        gl::clear( ColorA(0.0f,0.0f,0.0f,0.0f) );
        gl::enableAdditiveBlending();
        
        const float fadeSpeed = 0.005f;
        const float c = 1.0f - fadeSpeed;
        glColor4f(c, c, c, 1.0f);
        glEnable( GL_TEXTURE_2D );
        
        mFbo[mFboPong].bindTexture(0);
        gl::drawSolidRect(mFbo[mFboPing].getBounds());
        mFbo[mFboPong].unbindTexture();
        
        mFboScene.bindTexture(0);
        gl::drawSolidRect(mFbo[mFboPing].getBounds());
        mFboScene.unbindTexture();
    }
	mFbo[mFboPing].unbindFramebuffer();
    
    if(mEnableShader) 
    {
        mShader.bind();
        float invWidth = 1.0f/mFbo[mFboPong].getWidth();
        float invHeight = 1.0f/mFbo[mFboPong].getHeight();
        float i = 10.0f;//mBlurAmount;
        //mShader.uniform("tex0", 0);
        //mShader.uniform("blurCenterWeight", 0.5f);
        mShader.uniform("amountX", 1 * invWidth * i);
        mShader.uniform("amountY", 1 * invHeight * i);
        
    }
	
    if (mEnableShader)
    {
        //mShader.uniform("sampleOffset", Vec2f(0.0f, mBlurAmount));
	}
        
	mFbo[mFboPong].bindFramebuffer();
    {
        gl::clear( ColorA(0.0f,0.0f,0.0f,0.0f) );
        mFbo[mFboPing].bindTexture(0);
        gl::drawSolidRect(mFbo[mFboPong].getBounds());
        mFbo[mFboPing].unbindTexture();
    }
    mFbo[mFboPong].unbindFramebuffer();
        
    if (mEnableShader)
    {
        mShader.unbind();
    }
     */
}

void ShaderTest::draw()
{
    gl::pushMatrices();

    if( mUseFbo )
    {
        // Set up OpenGL
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_POLYGON_SMOOTH);
        glEnable(GL_POINT_SMOOTH);
        gl::enableAlphaBlending();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        gl::disableDepthRead();
        gl::disableDepthWrite();
        
        /*
        Area viewport = gl::getViewport();
        //gl::setMatricesWindow(mApp->getViewportSize());
        //gl::setViewport(mApp->getViewportBounds());
        
        // ****** DRAW SCENE ONTO FBO ******
        
        // Bind FBO to draw on it
        gl::setViewport(mFbo[mFboIndex].getBounds());
        mFbo[mFboIndex].bindFramebuffer();
        
        // Set up scene
        gl::clear(ColorAf(0.0f, 0.0f, 0.0f, 1.0f), true);
	    
        drawScene();
        
        // Unbind FBO
        mFbo[mFboIndex].unbindFramebuffer();
        
        // Advance FBO index
        mFboIndex = (mFboIndex + 1) % FBO_COUNT;
        
        // ****** DRAW GLSL MOTION BLUR ******
        
        // Set up window
        gl::clear(ColorAf::black());
        gl::setViewport(mApp->getViewportBounds());
        gl::setMatricesWindow(mApp->getViewportSize(), false);
        
        // Bind and configure shader
        mShader.bind();
        for (int32_t i = 0; i < FBO_COUNT; i++)
        {
            mFbo[i].bindTexture(i);
            mShader.uniform("tex" + toString(i), i);
        }
        
        // Draw shader output
        gl::color(Color::white());
        gl::drawSolidRect(mApp->getViewportBounds());
        
        // Unbind shader
        mShader.unbind();
        
        // Unbind FBOs
        for (int32_t i = 0; i < FBO_COUNT; i++)
        {
            mFbo[i].unbindTexture();
        }
        gl::setViewport( viewport );
        */
        
        mMotionBlurRenderer.draw();
    }
    else
    {
        drawScene();
    }
    
    gl::popMatrices();
}

void ShaderTest::drawScene()
{
    gl::setMatricesWindowPersp(mApp->getViewportSize());
    glColor4f(mColor);
    gl::drawSolidRect( Rectf(mPos.x, mPos.y, mPos.x + mRadius, mPos.y + mRadius) );
}

bool ShaderTest::handleKeyDown(const KeyEvent& keyEvent)
{
    bool handled = true;
    
    switch (keyEvent.getChar()) 
    {
        case 'z':
            mUseFbo = !mUseFbo;
            break;
        default:
            handled = false;
            break;
    }
    
    return handled;    
}

void ShaderTest::handleMouseDrag( const ci::app::MouseEvent& event )
{
    mPos = event.getPos();
}

void ShaderTest::handleOscMessage( const ci::osc::Message& message )
{
    if( message.getNumArgs() == 2 ) 
    {
        if( osc::TYPE_FLOAT == message.getArgType(0) )
        {
            mPos.x = message.getArgAsFloat(0) * mApp->getViewportWidth();
        }
        if( osc::TYPE_FLOAT == message.getArgType(1) )
        {
            mPos.y = message.getArgAsFloat(1) * mApp->getViewportHeight();
        }        
        console() << "[osc test] x: " << mPos.x << " y: " << mPos.y << std::endl;
    }
}
