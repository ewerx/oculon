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

void ParsecLabels::update(const ci::Camera &cam, float distance, float screenWidth, float screenHeight)
{
    BOOST_FOREACH( ParsecLabels::Label* &label, mLabels )
    {
		label->update( cam, 1.0f, screenWidth, screenHeight );
	}
    
    static const float minimum = 0.25f;
	static const float maximum = 1.0f;
	static const float range = 10.0f;
    
	if( distance > range ) {
		mAttenuation = ci::lerp<float>( minimum, 0.0f, (distance - range) / range );
		mAttenuation = math<float>::clamp( mAttenuation, 0.0f, maximum );
	}
	else {
		mAttenuation = math<float>::clamp( 1.0f - math<float>::log10( distance ) / math<float>::log10(range), minimum, maximum );
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
        label->draw( alpha * mAttenuation );
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
		//layout.addLine( spectrum );
		mNameTex = gl::Texture( layout.render( true, false ) );
		mSphere.setCenter( mPos );
		mSphere.setRadius( radius );
	}
}

void ParsecLabels::Label::update( const Camera &cam, float scale, float screenWidth, float screenHeight )
{
	mPos		= mInitPos * scale;
//	mSphere.setCenter( mPos );
	
	mScreenPos		= cam.worldToScreen( mPos, screenWidth, screenHeight );
	mDistToCam		= -cam.worldToEyeDepth( mPos );
//	mDistToCamPer	= math<float>::min( mDistToCam * 0.01f, 1.0f );
//	mScreenRadius	= cam.getScreenRadius( mSphere, screenWidth, screenHeight );
}

void ParsecLabels::Label::draw( float alpha )
{
	if( mDistToCam > 0.0f && mNameTex )
    {
		float alphaScale = 0.0f;
		
        if( mAudioPer > 0.0f )
        {
            alphaScale = mAudioPer * constrain( 1.95f - mDistToCam * 0.0000375f, 0.25f, 1.0f );
        }
        else
        {
            alphaScale = constrain( 1.0f - mDistToCam * 0.0000375f, 0.0f, 1.0f );
            alphaScale *= alphaScale * alphaScale;
        }
		
		if( mIsSelected )
			alphaScale = 1.0f;
		
		if( alphaScale > 0.05f )
        {
			gl::color( ColorA( 1.0f, 1.0f, 1.0f, alphaScale * alpha ) );
			//mNameTex.enableAndBind();
			gl::draw( mNameTex, mScreenPos + Vec2f( mScreenRadius + 35.0f, -28.0f ) );
			//mNameTex.disable();
            
            // indicator line
            const float hOffset = 35.0f;
            const float vOffset = -13.0f;
			gl::color( ColorA( 1.0f, 1.0f, 1.0f, 0.4f * alphaScale * alpha ) );
			Vec2f p1 = mScreenPos;
			Vec2f p2 = p1 + Vec2f( mScreenRadius + hOffset, vOffset );
			Vec2f p3 = mScreenPos + Vec2f( mScreenRadius + hOffset + mNameTex.getWidth(), vOffset );
			gl::drawLine( p1, p2 );
			gl::drawLine( p2, p3 );
		}
	}
}
