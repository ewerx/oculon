/*
 *  TextEntity.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-24.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "TextEntity.h"
#include "cinder/Text.h"
#include "cinder/gl/Texture.h"

using namespace std;
using namespace ci;

TextEntity::TextEntity()
: mString("")
, mFontName("Arial-BoldMT")
, mFontSize(12.0f)
, mTextColor(ColorA(1.0f,1.0f,1.0f,1.0f))
{
}

TextEntity::~TextEntity()
{
}

void TextEntity::update(double dt)
{
}

void TextEntity::draw()
{
    if( !mTexture )
        return;

    gl::pushMatrices();
    {
        
        glDisable( GL_LIGHTING );
        glDisable( GL_DEPTH_TEST );
        //gl::enableAdditiveBlending();
        glEnable( GL_TEXTURE_2D );
        glColor4f( mTextColor );
        
        
        glTranslatef(mPosition);
        gl::draw( mTexture, Rectf( 0.0f, 0.0f, mTexture.getCleanWidth(), mTexture.getCleanHeight() ));
        
        glDisable( GL_TEXTURE_2D );
        glEnable( GL_DEPTH_TEST );
        //gl::enableAlphaBlending();
        glEnable( GL_LIGHTING );
    }
    gl::popMatrices();
}

void TextEntity::setText( const std::string& str )
{
    mString = str;
    createTexture();
}

void TextEntity::setText( const std::string& str, const ci::ColorA& color )
{
    mTextColor = color;
    setText(str);
}

void TextEntity::setText( const std::string& str, const std::string& font, const float size, const ci::ColorA& color )
{
    mFontName = font;
    mFontSize = size;
    mTextColor = color;
    setText(str);
}

void TextEntity::setTextColor( const ci::ColorA& color )
{
    mTextColor = color;
    createTexture();
}

void TextEntity::setFont( const std::string& fontName )
{
    mFontName = fontName;
    createTexture();
}

void TextEntity::createTexture()
{
    TextLayout layout;
    
    layout.setFont( Font( mFontName, mFontSize ) );
    layout.setColor( mTextColor );
    layout.addLine( mString );
    
    mTexture = gl::Texture( layout.render( true ) ); //TODO: check for memory leak??
}
