//
//  TextureGenerator.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 11/22/2013.
//
//

#include "TextureGenerator.h"

using namespace ci;

TextureGenerator::TextureGenerator()
{
}

TextureGenerator::~TextureGenerator()
{
}

void TextureGenerator::setup(const int width, const int height)
{
    initShader();
    
    // init FBO
    gl::Fbo::Format format;
	format.setColorInternalFormat( GL_RGB32F_ARB );
	mFbo = gl::Fbo( width, height, format );
    
    // initialize
    mFbo.bindFramebuffer();
	gl::setViewport( mFbo.getBounds() );
	gl::clear();
	mFbo.unbindFramebuffer();
	mFbo.getTexture().setWrap( GL_REPEAT, GL_REPEAT );
}

void TextureGenerator::update(double dt)
{
    gl::pushMatrices();
    
    // Bind FBO and set up window
	mFbo.bindFramebuffer();
	gl::setViewport( mFbo.getBounds() );
	gl::setMatricesWindow( mFbo.getSize() );
	gl::clear();
    
	// Bind and configure dynamic texture shader
	mShader.bind();
    configShader(dt);
    
	// Draw shader output
	gl::enable( GL_TEXTURE_2D );
	gl::color( Colorf::white() );
	gl::begin( GL_TRIANGLES );
    
    // TODO: cleanup
	// Define quad vertices
	Vec2f vert0( (float)mFbo.getBounds().x1, (float)mFbo.getBounds().y1 );
	Vec2f vert1( (float)mFbo.getBounds().x2, (float)mFbo.getBounds().y1 );
	Vec2f vert2( (float)mFbo.getBounds().x1, (float)mFbo.getBounds().y2 );
	Vec2f vert3( (float)mFbo.getBounds().x2, (float)mFbo.getBounds().y2 );
    
	// Define quad texture coordinates
	Vec2f uv0( 0.0f, 0.0f );
	Vec2f uv1( 1.0f, 0.0f );
	Vec2f uv2( 0.0f, 1.0f );
	Vec2f uv3( 1.0f, 1.0f );
    
	// Draw quad (two triangles)
	gl::texCoord( uv0 );
	gl::vertex( vert0 );
	gl::texCoord( uv2 );
	gl::vertex( vert2 );
	gl::texCoord( uv1 );
	gl::vertex( vert1 );
    
	gl::texCoord( uv1 );
	gl::vertex( vert1 );
	gl::texCoord( uv2 );
	gl::vertex( vert2 );
	gl::texCoord( uv3 );
	gl::vertex( vert3 );
    
	gl::end();
    gl::disable( GL_TEXTURE_2D );
    
	// Unbind everything
	mShader.unbind();
	mFbo.unbindFramebuffer();
    
    gl::popMatrices();
}
