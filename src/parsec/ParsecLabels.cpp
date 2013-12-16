//
//  ParsecLabels.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 12/12/2013.
//
//

#include "ParsecLabels.h"
#include "OculonApp.h"
#include "cinder/Text.h"

#include <boost/foreach.hpp>

using namespace ci;

#pragma mark - ParsecLabels

ParsecLabels::ParsecLabels()
{
}

ParsecLabels::~ParsecLabels()
{
    BOOST_FOREACH( ParsecLabels::Label* &label, mLabels )
    {
		delete label;
	}
}

void ParsecLabels::addLabel(ParsecLabels::Label *label)
{
    mLabels.push_back(label);
}

void ParsecLabels::update(const ci::Camera &cam, float screenWidth, float screenHeight)
{
    BOOST_FOREACH( ParsecLabels::Label* &label, mLabels )
    {
		label->update( cam, 0.2f, screenWidth, screenHeight );
	}
}

void ParsecLabels::draw(float screenWidth, float screenHeight, float alpha)
{
    gl::setMatricesWindow(Vec2i(screenWidth,screenHeight), true );
    
    gl::enableAlphaBlending();
    // black box to darken the stars...
    gl::color(0.0f,0.0f,0.0f, 1.0f-alpha);
    gl::drawSolidRect( Area(0,0,screenWidth,screenHeight) );
    
    // TODO: try alpha..
    gl::enableAlphaBlending();

    gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
    
    BOOST_FOREACH( ParsecLabels::Label* &label, mLabels )
    {
        label->draw( alpha );
    }
}

#pragma mark - ParsecLabels::Label

ParsecLabels::Label::Label( Vec3f pos, float absMag, std::string name, std::string spectrum, const Font &font )
: mPos( pos ), mName( name )
{
	mInitPos		= mPos;
	mIsSelected		= false;
    mAudioPer       = 0.0f;
    
    float radius = 0.01f;//( 10.0f - absMag ) * 0.025f;
    mScreenRadius	= 0.01f;
	
	if( mName.length() > 1 )
    {
		TextLayout layout;
		layout.clear( ColorA( 0.0f, 0.0f, 0.0f, 0.0f ) );
		layout.setFont( font );
		layout.setColor( Color( 1.0f, 1.0f, 1.0f ) );
		layout.addLine( name );
		layout.setFont( font );
		layout.setLeadingOffset( 3 );
		layout.addLine( spectrum );
		mNameTex = gl::Texture( layout.render( true, false ) );
		mSphere.setCenter( mPos );
		mSphere.setRadius( radius );
	}
}

void ParsecLabels::Label::update( const Camera &cam, float scale, float screenWidth, float screenHeight )
{
	mPos		= mInitPos * scale;
	mSphere.setCenter( mPos );
	
	mScreenPos		= cam.worldToScreen( mPos, screenWidth, screenHeight );
	mDistToCam		= -cam.worldToEyeDepth( mPos );
	mDistToCamPer	= math<float>::min( mDistToCam * 0.01f, 1.0f );
	mScreenRadius	= 0.01f;//cam.getScreenRadius( mSphere, screenWidth, screenHeight );
}

void ParsecLabels::Label::draw( float alpha )
{
	if( mDistToCam > 0.0f && mNameTex )
    {
		float per = 0.0f;
		
        if( mAudioPer > 0.0f )
        {
            per = mAudioPer * constrain( 1.95f - mDistToCam * 0.0000375f, 0.25f, 1.0f );
        }
        else
        {
            per = constrain( 1.0f - mDistToCam * 0.0000375f, 0.0f, 1.0f );
            per *= per * per;
        }
		
		if( mIsSelected )
			per = 1.0f;
		
		if( per > 0.05f ){
			gl::color( ColorA( 1.0f, 1.0f, 1.0f, per * alpha ) );
			mNameTex.enableAndBind();
			gl::draw( mNameTex, mScreenPos + Vec2f( mScreenRadius + 35.0f, -28.0f ) );
			mNameTex.disable();
            
			gl::color( ColorA( 1.0f, 1.0f, 1.0f, per * 0.4f * alpha ) );
			Vec2f p1 = mScreenPos;
			Vec2f p2 = p1 + Vec2f( mScreenRadius + 35.0f, -13.0f );
			Vec2f p3 = mScreenPos + Vec2f( mScreenRadius + 35.0f + mNameTex.getWidth(), -13.0f );
			gl::drawLine( p1, p2 );
			gl::drawLine( p2, p3 );
		}
	}
}
