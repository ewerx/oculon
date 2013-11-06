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
{
}

InfoPanel::~InfoPanel()
{
}

void InfoPanel::init()
{
    mTextureFont = gl::TextureFont::create( Font( "Arial", 10 ) );
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
    if( mOpacity > 0.01f )
    {
        glDisable( GL_LIGHTING );
        
        gl::disableDepthRead();
        gl::disableDepthWrite();
        gl::enableAlphaBlending();
        
        gl::pushMatrices();
        gl::setMatricesWindow( aWindowDim );
        
        float w = 70.0f;
        float h = 10.0f * mLines.size();
        
        float x = aWindowDim.x - w - 20.0f;
        float y = (aWindowDim.y - h) - 20.0f;
        
        glDisable( GL_TEXTURE_2D );
        const float pad = 4.0f;
        gl::color( 0.1, 0.1, 0.1, mOpacity*0.8f );
        gl::drawSolidRect( Rectf( x-pad, y-pad, x+w+pad, y+h+pad ) );
        
        glEnable( GL_TEXTURE_2D );
        gl::color( 1.0f, 1.0f, 1.0f, mOpacity );
        
        y += 8.0f;
        
        for (std::vector<InfoLine>::iterator lineIt = mLines.begin();
             lineIt != mLines.end();
             ++lineIt)
        {
            gl::color( (*lineIt).mColor );
            mTextureFont->drawString((*lineIt).mString, Vec2f(x,y) );
            y += 10.0f;
        }
        mLines.clear();
        
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

