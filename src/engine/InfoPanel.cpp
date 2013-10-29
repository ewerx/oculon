/*
 *  InfoPanel.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "InfoPanel.h"
#include "cinder/Text.h"
#include "cinder/gl/Texture.h"

using namespace ci;


InfoPanel::InfoPanel()
: mIsVisible(true)
, mOpacity(1.0f)
, mTextureReady(false)
{
}

InfoPanel::~InfoPanel()
{
}

void InfoPanel::createTexture()
{
    if( mLines.size() > 0 && mIsVisible )
    {
        TextLayout layout;
        
        //layout.setFont( Font( "Arial-BoldMT", 12 ) );
        //layout.setColor( Color( 1.0f, 1.0f, 0.0f ) );
        //layout.addLine( "OpenGL Lighting" );
        
        layout.setFont( Font( "ArialMT", 10 ) );
        layout.setColor( Color( 1.0f, 0.7f, 0.0f ) );
        
        for (std::vector<InfoLine>::iterator lineIt = mLines.begin();
             lineIt != mLines.end(); 
             ++lineIt) 
        {
            layout.setColor( (*lineIt).mColor );
            layout.addLine( (*lineIt).mString );
        }
        
        mTexture = gl::Texture( layout.render( true ) );
        
        mTextureReady = true;
        mLines.clear();
    }
    else
    {
        mTextureReady = false;
    }
}

void InfoPanel::addLine( const std::string& line, const ci::Color& color )
{
    InfoLine infoLine;
    infoLine.mString = line;
    infoLine.mColor = color;
    mLines.push_back(infoLine);
}


void InfoPanel::update()
{
    createTexture();
    
	if( mIsVisible && mOpacity < 1.0f )
    {
		mOpacity -= ( mOpacity - 1.0f ) * 0.1f;
	} 
    else if( !mIsVisible && mOpacity > 0.0f )
    {
		mOpacity -= ( mOpacity - 0.0f ) * 0.1f;	
	}
}


void InfoPanel::render( Vec2f aWindowDim )
{
    if( mOpacity > 0.01f && mTextureReady )
    {
        glDisable( GL_LIGHTING );
        
        gl::disableDepthRead();
        gl::disableDepthWrite();
        gl::enableAlphaBlending();
        
        gl::pushMatrices();
        gl::setMatricesWindow( aWindowDim );
        
        float x = aWindowDim.x - mTexture.getWidth() - 20.0f;
        float y = aWindowDim.y - mTexture.getHeight() - 20.0f;
        
        glDisable( GL_TEXTURE_2D );
        const float pad = 4.0f;
        glColor4f( 0.1, 0.1, 0.1, mOpacity*0.8f );
        gl::drawSolidRect( Rectf( x-pad, y-pad, x+mTexture.getWidth()+pad, y+mTexture.getHeight()+pad ) );
        
        glEnable( GL_TEXTURE_2D );
        glColor4f( 1, 1, 1, mOpacity );
        gl::draw( mTexture, Vec2f( x, y ) );
        
        gl::popMatrices();
        
        glDisable( GL_TEXTURE_2D );
        gl::enableDepthRead();
        gl::enableDepthWrite();
    }
}


void InfoPanel::toggleState()
{
	mIsVisible = ! mIsVisible;
    //mOpacity = 1.0f;
}

