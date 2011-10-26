/*
 *  InfoPanel.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __INFOPANEL_H__
#define __INFOPANEL_H__

#include "cinder/Vector.h"
#include "cinder/gl/Texture.h"
#include "cinder/Color.h"
#include <vector>
#include <string>

using namespace std;


struct InfoLine
{
    string mString;
    ci::Color mColor;
};

class InfoPanel {
public:
	InfoPanel();
    virtual ~InfoPanel();
    
	void update();
	void render( ci::Vec2f aWindowDim );
	void toggleState();// TODO: make this an event listener
    
    void addLine( const string& line, const ci::Color& color );
    
protected:
    void createTexture();
	
private:
	float				mOpacity;
	bool				mState;
	ci::gl::Texture     mTexture;
    bool                mTextureReady;
    
    vector<InfoLine>      mLines;
};

#endif // __INFOPANEL_H__
