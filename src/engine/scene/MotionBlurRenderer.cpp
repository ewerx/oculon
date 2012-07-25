/*
 *  MotionBlurRenderer.cpp
 *  Oculon
 *
 *  Created by Ehsan on 12-Apr-03.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "OculonApp.h"//TODO: fix this dependency
#include "MotionBlurRenderer.h"
#include "Resources.h"
#include "cinder/Utilities.h"

using namespace ci;

MotionBlurRenderer::MotionBlurRenderer()
{
}

MotionBlurRenderer::~MotionBlurRenderer()
{
}

void MotionBlurRenderer::setup(const Vec2i& windowSize, MotionBlurRenderer::tDrawCallback drawCallback)
{
    mDrawSceneCallback = drawCallback;
    mWindowSize = windowSize;
    
    // setup FBO
    gl::Fbo::Format format;
    const int fboWidth = windowSize.x;
    const int fboHeight = windowSize.y;
    //format.enableMipmapping(false);
    format.enableDepthBuffer(false);
	format.setCoverageSamples(8);
	format.setSamples(4); // 4x AA
    
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
}

void MotionBlurRenderer::resize(const Vec2i& windowSize)
{
    
}

void MotionBlurRenderer::draw()
{
    Area viewport = gl::getViewport();
    //gl::setMatricesWindow(getWindowSize());
    //gl::setViewport(getWindowBounds());
    
    /****** DRAW SCENE ONTO FBO ******/
    
    // Bind FBO to draw on it
    gl::setViewport(mFbo[mFboIndex].getBounds());
    mFbo[mFboIndex].bindFramebuffer();
    
    // Set up scene
    gl::clear(ColorAf(0.0f, 0.0f, 0.0f, 1.0f), true);
    
    mDrawSceneCallback();
    
    // Unbind FBO
    mFbo[mFboIndex].unbindFramebuffer();
    
    // Advance FBO index
    mFboIndex = (mFboIndex + 1) % FBO_COUNT;
    
    /****** DRAW GLSL MOTION BLUR ******/
    
    // Set up window
    gl::clear(ColorAf::black());
    gl::setViewport(Area(0,0,mWindowSize.x,mWindowSize.y));
    gl::setMatricesWindow(mWindowSize, false);
    
    // Bind and configure shader
    mShader.bind();
    for (int32_t i = 0; i < FBO_COUNT; i++)
    {
        mFbo[i].bindTexture(i);
        mShader.uniform("tex" + toString(i), i);
    }
    
    // Draw shader output
    gl::color(Color::white());
    gl::drawSolidRect(Area(0,0,mWindowSize.x,mWindowSize.y));
    
    // Unbind shader
    mShader.unbind();
    
    // Unbind FBOs
    for (int32_t i = 0; i < FBO_COUNT; i++)
    {
        mFbo[i].unbindTexture();
    }
    gl::setViewport( viewport );
}
