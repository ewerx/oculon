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
#include "cinder/Text.h"
#include <string>
#include "Entity.h"

class Scene;

class TextEntity : public Entity<float>
{
public:
    TextEntity(Scene* scene);
    virtual ~TextEntity();
    
    // inherited from Entity
    void update(double dt);
    void draw();
    
    void resize(); //TODO: make this part of Entity
    
    // new
    const std::string& getText() { return mTextBox.getText(); }
    void setText( const std::string& str );
    void setText( const std::string& str, const ci::ColorA& color );
    void setText( const std::string& str, const std::string& font, const float size, const ci::ColorA& color );
    void setTextColor( const ci::ColorA& color );
    void setFont( const std::string& fontName, const float fontSize );
    void setRightJustify( bool on, float margin = 0.0f );
    void setBottomJustify( bool on, float margin = 0.0f )   { mJustifyBottom = on; mMarginBottom = margin; }
    
private:
    void updateTextSize();
    
private:
    std::string         mString;
    TextBox             mTextBox;
    bool                mJustifyRight;
    bool                mJustifyBottom;
    float               mMarginRight;
    float               mMarginBottom;
};

#endif // __TEXTENTITY_H__
