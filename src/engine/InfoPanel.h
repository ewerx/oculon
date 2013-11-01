/*
 *  InfoPanel.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __INFOPANEL_H__
#define __INFOPANEL_H__

#include "cinder/Vector.h"
#include "cinder/gl/Texture.h"
#include "cinder/Color.h"
#include "cinder/gl/TextureFont.h"
#include <vector>
#include <string>


struct InfoLine
{
    std::string mString;
    ci::Color mColor;
};

class InfoPanel {
public:
	InfoPanel();
    virtual ~InfoPanel();
    
    void init();
	void update();
	void render( ci::Vec2f aWindowDim );
	void toggleState();// TODO: make this an event listener
    void setVisible(bool visible)   { mIsVisible = visible; }
    bool isVisible() const          { return mIsVisible; }
    
    void addLine( const std::string& line, const ci::Color& color );
    
protected:
    //void createTexture();
	
private:
	float				mOpacity;
	bool				mIsVisible;
    
    ci::gl::TextureFontRef mTextureFont;
    
    std::vector<InfoLine>      mLines;
};

#endif // __INFOPANEL_H__
