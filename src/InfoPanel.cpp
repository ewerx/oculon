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
: mState(true)
, mOpacity(1.0f)
, mTextureReady(false)
{
}

InfoPanel::~InfoPanel()
{
}

void InfoPanel::createTexture()
{
    if( mLines.size() > 0 )
    {
        TextLayout layout;
        
        //layout.setFont( Font( "Arial-BoldMT", 12 ) );
        //layout.setColor( Color( 1.0f, 1.0f, 0.0f ) );
        //layout.addLine( "OpenGL Lighting" );
        
        layout.setFont( Font( "ArialMT", 10 ) );
        layout.setColor( Color( 1.0f, 0.7f, 0.0f ) );
        
        for (vector<InfoLine>::iterator lineIt = mLines.begin(); 
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

void InfoPanel::addLine( const string& line, const Color& color )
{
    InfoLine infoLine;
    infoLine.mString = line;
    infoLine.mColor = color;
    mLines.push_back(infoLine);
}


void InfoPanel::update()
{
    createTexture();
    
	if( mState && mOpacity < 1.0f )
    {
		mOpacity -= ( mOpacity - 1.0f ) * 0.1f;
	} 
    else if( !mState && mOpacity > 0.0f )
    {
		mOpacity -= ( mOpacity - 0.0f ) * 0.1f;	
	}
}


void InfoPanel::render( Vec2f aWindowDim )
{
    if( mOpacity > 0.01f && mTextureReady)
    {
        glDisable( GL_LIGHTING );
        glEnable( GL_TEXTURE_2D );
        glColor4f( 1, 1, 1, 1 );
        
        gl::pushMatrices();
        gl::setMatricesWindow( aWindowDim );
        
        float x = aWindowDim.x - mTexture.getWidth() - 20.0f;
        float y = aWindowDim.y - mTexture.getHeight() - 40.0f;
        glColor4f( 1, 1, 1, mOpacity );
        gl::draw( mTexture, Vec2f( x, y ) );
        
        gl::popMatrices();
        
        glDisable( GL_TEXTURE_2D );
    }
}


void InfoPanel::toggleState()
{
	mState = ! mState;
    //mOpacity = 1.0f;
}

