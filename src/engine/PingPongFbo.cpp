//
//  PingPongFbo
//
//  Created by Éric Renaud-Houde on 2013-01-06.
//
//

#include "PingPongFbo.h"
#include <assert.h>
#include <utility>

using namespace ci;
using namespace std;


PingPongFbo::PingPongFbo( const std::vector<Surface32f>& surfaces )
: mCurrentFbo(0)
{
	init(surfaces);
	reset();
}

PingPongFbo::PingPongFbo( const int numTextures, const int size )
: mCurrentFbo(0)
{
    // create and populate surfaces
    vector<Surface32f> surfaces;
    for (int i = 0; i < numTextures; ++i)
    {
        surfaces.push_back( Surface32f(size,size,true) );
    }
    
    Surface32f::Iter iterator = surfaces.front().getIter();
    while(iterator.line())
	{
		while(iterator.pixel())
		{
            for (Surface32f& s : surfaces)
            {
                s.setPixel(iterator.getPos(), ColorA::black());
            }
		}
	}
    
    init(surfaces);
    reset();
}

void PingPongFbo::init( const std::vector<Surface32f>& surfaces )
{
    if( surfaces.empty() ) return;
	
	int i = 0;
	mTextureSize = surfaces[0].getSize();
	for( const Surface32f& s : surfaces) {
		mAttachments.push_back(GL_COLOR_ATTACHMENT0_EXT + i);
		addTexture(s);
		i++;
	}
	
	int max =gl::Fbo::getMaxAttachments();
	std::cout << "[PINGPONG] setting up FBOs with " << surfaces.size() << " attachments. max: " << max << std::endl;
	assert(surfaces.size() < max);
	
	gl::Fbo::Format format;
	format.enableDepthBuffer(false);
	format.enableColorBuffer(true, mAttachments.size());
	format.setMinFilter( GL_NEAREST );
	format.setMagFilter( GL_NEAREST );
	format.setColorInternalFormat( GL_RGBA32F_ARB );
	mFbos[0] = gl::Fbo( mTextureSize.x, mTextureSize.y, format );
	mFbos[1] = gl::Fbo( mTextureSize.x, mTextureSize.y, format );
}

void PingPongFbo::addTexture(const Surface32f &surface)
{
    assert(mTextures.size() < mAttachments.size());
    assert(surface.getSize() == mTextureSize);
    
    gl::Texture::Format format;
    format.setInternalFormat( GL_RGBA32F_ARB );
	gl::Texture tex = gl::Texture( surface, format );
    tex.setWrap( GL_REPEAT, GL_REPEAT );
    tex.setMinFilter( GL_NEAREST );
    tex.setMagFilter( GL_NEAREST );
    mTextures.push_back( tex );
}

void PingPongFbo::setTextures(const std::vector<ci::Surface32f> &surfaces)
{
    assert( surfaces.size() == mTextures.size() );
    if (surfaces.size() != mTextures.size()) return;
    
    mTextures.clear();
	for( const Surface32f& s : surfaces)
    {
        addTexture(s);
    }
    
    reset();
}

void PingPongFbo::setTexture(const int attachment, const ci::Surface32f &surface)
{
    assert( attachment < mTextures.size() );
    
    gl::Texture::Format format;
    format.setInternalFormat( GL_RGBA32F_ARB );
	gl::Texture tex = gl::Texture( surface, format );
    tex.setWrap( GL_REPEAT, GL_REPEAT );
    tex.setMinFilter( GL_NEAREST );
    tex.setMagFilter( GL_NEAREST );
    mTextures[attachment] = tex;
}

void PingPongFbo::setTexture(const int attachment, const ci::gl::Texture &texture)
{
    assert( attachment < mTextures.size() );
    
    mTextures[attachment] = texture;
}

void PingPongFbo::reset()
{
    // redraws the textures into the FBO
	mFbos[mCurrentFbo].bindFramebuffer();
    gl::setMatricesWindow( getSize(), false );
    gl::setViewport( getBounds() );
    gl::clear();
    for(int i=0; i<mAttachments.size(); ++i) {
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + i);
        
        mTextures[i].enableAndBind();
        gl::draw( mTextures[i], getBounds() );
        mTextures[i].unbind();
        mTextures[i].disable();
    }
    mFbos[mCurrentFbo].unbindFramebuffer();
	
	mFbos[!mCurrentFbo] = mFbos[mCurrentFbo];
}

void PingPongFbo::swap()
{
    mCurrentFbo = !mCurrentFbo;
}

void PingPongFbo::bindUpdate()
{
    mFbos[ mCurrentFbo ].bindFramebuffer();
    
    glDrawBuffers(mAttachments.size(), &mAttachments[0]);
    
    for(int i=0; i<mAttachments.size(); ++i) {
        mFbos[!mCurrentFbo].bindTexture(i, i);
    }
}

void PingPongFbo::unbindUpdate()
{
    mFbos[ !mCurrentFbo ].unbindTexture();
    mFbos[ mCurrentFbo ].unbindFramebuffer();
	
	swap();
}

void PingPongFbo::bindTexture(int textureUnit)
{
    if (textureUnit < mAttachments.size()) {
        mFbos[mCurrentFbo].bindTexture(textureUnit, textureUnit);
    } else {
        assert(false && "invalid textureUnit");
    }
}

void PingPongFbo::unbindTexture()
{
    mFbos[mCurrentFbo].unbindTexture();
}

gl::Texture& PingPongFbo::getTexture(int attachment)
{
    return mFbos[mCurrentFbo].getTexture(attachment);
}

Vec2i PingPongFbo::getSize() const
{
    return mTextureSize;
}


Area PingPongFbo::getBounds() const
{
    return mFbos[0].getBounds();
}
