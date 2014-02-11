//
//  DomeRenderer.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2013-01-27.
//
//

#include "DomeRenderer.h"
#include "MeshHelper.h"

using namespace ci;

DomeRenderer::DomeRenderer()
{
    
}

void DomeRenderer::setup(int width, int height)
{
    gl::Fbo::Format format;
    //format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
    mDomeFbo = gl::Fbo( width, height, format );
    
    int numSegments = 128;
	mSphere	= gl::VboMesh( MeshHelper::createSphere( Vec2i(numSegments,numSegments) ) );
    
    mScale = Vec3f::one();
}

void DomeRenderer::renderFboToDome(ci::gl::Fbo &fbo, const ci::Camera& camera)
{
    mDomeFbo.bindFramebuffer();
    
	gl::pushMatrices();
    //gl::setMatricesWindow( fbo.getSize(), false );
	gl::setViewport( fbo.getBounds() );
    gl::setMatrices( camera );
	gl::clear( ColorAf::black() );
    
    gl::enable( GL_TEXTURE_2D );
    fbo.getTexture().bind();
    
    // Flip, scale
	gl::rotate( Vec3f( 0.0f, 180.0f, 180.0f ) ); // Y is flipped for rear projection
	gl::scale( mScale );
    
    // Draw shere
	gl::draw( mSphere );
	
	// End scale
	gl::popMatrices();
    
    fbo.getTexture().unbind();
    gl::disable( GL_TEXTURE_2D );
    
    mDomeFbo.unbindFramebuffer();
}