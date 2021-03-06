/*
 *  TextEntity.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-24.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "TextEntity.h"
#include "OculonApp.h"
#include "cinder/Text.h"
#include "cinder/gl/Texture.h"
#include "cinder/app/App.h"
#include "Scene.h"

using namespace std;
using namespace ci;

TextEntity::TextEntity(Scene* scene)
: Entity<float>(scene,Vec3f::zero())
, mString("")
, mJustifyRight(false)
, mJustifyBottom(false)
, mMarginRight(10.0f)
, mMarginBottom(10.0f)
{
}

TextEntity::~TextEntity()
{
}

void TextEntity::resize()
{
    updateTextSize();
}

void TextEntity::update(double dt)
{
}

void TextEntity::draw()
{
    if( mTextBox.getText().length() == 0 )
    {
        return;
    }

    gl::pushMatrices();
    {
        
        glDisable( GL_LIGHTING );
        glDisable( GL_DEPTH_TEST );
        //gl::enableAdditiveBlending();
        glEnable( GL_TEXTURE_2D );
        glColor4f( mTextBox.getColor() );
        
        
        glTranslatef(mPosition);
        //gl::draw( mTexture, Rectf( 0.0f, 0.0f, mTexture.getCleanWidth(), mTexture.getCleanHeight() ));
        gl::draw( mTextBox.render() );
        
        //glDisable( GL_TEXTURE_2D );
        glEnable( GL_DEPTH_TEST );
        //gl::enableAlphaBlending();
        //glEnable( GL_LIGHTING );
    }
    gl::popMatrices();
}

void TextEntity::setText( const std::string& str )
{
    mTextBox.setText(str);
    updateTextSize();
}

void TextEntity::setText( const std::string& str, const ci::ColorA& color )
{
    setText(str);
    mTextBox.setColor(color);
}

void TextEntity::setText( const std::string& str, const std::string& font, const float size, const ci::ColorA& color )
{
    setText(str);
    mTextBox.setColor(color);
    mTextBox.setFont(Font( font, size ));
}

void TextEntity::setTextColor( const ci::ColorA& color )
{
    mTextBox.setColor(color);
}

void TextEntity::setFont( const std::string& fontName, const float fontSize )
{
    mTextBox.setFont(Font(fontName,fontSize));
    updateTextSize();
}

void TextEntity::updateTextSize()
{
    if( mJustifyRight )
    {
        mPosition.x = mParentScene->getApp()->getViewportWidth() - mTextBox.measure().x - mMarginRight;
    }
    
    if( mJustifyBottom )
    {
        mPosition.y = mParentScene->getApp()->getViewportHeight() - mTextBox.measure().y - mMarginBottom;
    }
}

void TextEntity::setRightJustify( bool on, float margin )
{
    mJustifyRight = on; 
    mMarginRight = margin;
    //mTextBox.setAlignment( mJustifyRight ? TextBox::RIGHT : TextBox::LEFT );//TODO: doesn't work...
}
