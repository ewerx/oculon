/*
 *  TextEntity.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-24.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __TEXTENTITY_H__
#define __TEXTENTITY_H__


#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/gl/Texture.h"
#include <string>
#include "Entity.h"

class TextEntity : public Entity<float>
{
public:
    TextEntity();
    virtual ~TextEntity();
    
    // inherited from Entity
    void update(double dt);
    void draw();
    
    // new
    void setText( const std::string& str );
    void setText( const std::string& str, const ci::ColorA& color );
    void setText( const std::string& str, const std::string& font, const float size, const ci::ColorA& color );
    void setTextColor( const ci::ColorA& color );
    
private:
    void createTexture();
    
private:
    std::string         mString;
    std::string         mFontName;
    float               mFontSize;
    ci::ColorA          mTextColor;
    ci::gl::Texture     mTexture;
};

#endif // __TEXTENTITY_H__
