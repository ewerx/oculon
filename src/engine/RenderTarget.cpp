//
//  RenderTarget.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-12-12.
//
//

#include "RenderTarget.h"

using namespace oculon;
using namespace cinder;
/*
RenderTarget::RenderTarget( int32_t width, int32_t height )
{
    setupFbo(width, height);
}

void RenderTarget::setupFbo( int32_t width, int32_t height )
{
    gl::Fbo::Format format;
    format.enableMipmapping(false);
    format.enableDepthBuffer(false);
    format.setCoverageSamples(8);
    format.setSamples(4); // 4x AA
    
    mFbo = gl::Fbo( width, height, format );
    
    clearFbo();
    
    mFbo.getTexture().setFlipped(true);
}

void RenderTarget::clearFbo()
{
    // clear the buffer
    mFbo.bindFramebuffer();
    {
        gl::pushMatrices();
        {
            gl::setMatricesWindow( mFbo.getSize() );
            gl::setViewport( mFbo.getBounds() );
            gl::clear( ColorA(0.0f,0.0f,0.0f,0.0f) );
        }
        gl::popMatrices();
    }
    mFbo.unbindFramebuffer();
}

void RenderTarget::addScene(SceneRef scene)
{
    mScenes.push_back(scene);
}

void RenderTarget::preRender()
{
    mFbo.bindFramebuffer();
}

void RenderTarget::renderScenes()
{
    
}

void RenderTarget::postRender()
{
    mFbo.unbindFramebuffer();
}
*/