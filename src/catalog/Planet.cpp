/*
 *  Planet.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-19.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */


#include "Planet.h"
#include "cinder/CinderMath.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/Text.h"
#include "cinder/app/AppBasic.h"
#include "Scene.h"
#include "OculonApp.h"

using namespace ci;

Planet::Planet( Scene* scene,
               Star* hostStar,
               std::string name,
               float orbitalPeriod,
               float semiMajorAxis,
               float inclination,
               float mass,
               float radius )
: mParentScene(scene)
, mHostStar(hostStar)
, mName( name )
, mOrbitalPeriod(orbitalPeriod)
, mSemiMajorAxis(semiMajorAxis)
, mInclination(inclination)
, mMass(mass)
, mRadius(radius)
{
    /*
	if( mName.length() > 1 && appMag < 6.0f ){
		TextLayout layout;
		layout.clear( ColorA( 0.0f, 0.0f, 0.0f, 0.0f ) );
		layout.setFont( fontM );
		layout.setColor( Color( 1.0f, 1.0f, 1.0f ) );
		layout.addLine( name );
		layout.setFont( fontS );
		layout.setLeadingOffset( 3 );
		layout.addLine( spectrum );
		mNameTex = gl::Texture( layout.render( true, false ) );
		mSphere.setCenter( mPos );
		mSphere.setRadius( mRadius );
	}
     */
}

void Planet::update( const Camera &cam, float scale )
{
}

/*
void Planet::drawName( const Vec2f &mousePos, float power, float alpha )
{
    
	if( mDistToCam > 0.0f && mNameTex ){
		float per = constrain( 1.0f - mDistToCam * 0.0000375f, 0.0f, 1.0f );
		per *= per * per;
		
		Vec2f dirToMouse	= mScreenPos - mousePos;
		mDistToMouse		= dirToMouse.length();
		
		if( mDistToMouse < 40.0f )
			per += 1.0f - mDistToMouse/40.0f;
		
		if( mIsSelected )
			per = 1.0f;
		
		if( per > 0.05f ){
			gl::color( ColorA( power, power, power, per * alpha ) );
			mNameTex.enableAndBind();
			gl::draw( mNameTex, mScreenPos + Vec2f( mScreenRadius + 35.0f, -28.0f ) );
			mNameTex.disable();
				
			gl::color( ColorA( power, power, power, per * 0.4f * alpha ) );
			Vec2f p1 = mScreenPos;
			Vec2f p2 = p1 + Vec2f( mScreenRadius + 35.0f, -13.0f );
			Vec2f p3 = mScreenPos + Vec2f( mScreenRadius + 35.0f + mNameTex.getWidth(), -13.0f );
			gl::drawLine( p1, p2 );
			gl::drawLine( p2, p3 );
		}
	}
     
}
*/