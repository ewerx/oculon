/*
 *  Quake.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */


#include "Quake.h"
#include "QuakeData.h"
#include "Utils.h"
#include "Scene.h"
#include "OculonApp.h"

#include "cinder/gl/gl.h"
#include "cinder/CinderMath.h"
#include "cinder/Rand.h"
#include "cinder/Easing.h"
#include "cinder/Camera.h"

using namespace ci;
using namespace std;


//
// Quake
// 

Quake::Quake(Scene* scene, const QuakeEvent* data, const int mapOffset)
: Entity<float>(scene,Vec3f::zero())
, mLabel(scene)
, mEventData(NULL)
, mShowLabel(true)
, mState(STATE_INVALID)
{
    mLabel.setPosition(Vec3f(10.0f, 0.0f, 0.0f));
    mLabel.setFont("Menlo", 10.0f);
    mLabel.setTextColor( ColorA(1.0f,1.0f,1.0f,0.95f) );
    
    setEvent(data, mapOffset);
}

Quake::~Quake()
{
}

void Quake::setup()
{
}

void Quake::setEvent( const QuakeEvent* data, const int mapOffset )
{
    mEventData = data;
    mState = STATE_IDLE;
    
    // map location
    Vec2f mapPos = Utils::toEquirectProjection( data->getLat(), data->getLong(), mParentScene->getApp()->getViewportWidth(), mParentScene->getApp()->getViewportHeight(), mapOffset );
    mPosition.x = mapPos.x;
    mPosition.y = mapPos.y;
    mPosition.z = 0.0f;
    
    mLabel.setPosition(Vec3f((int)(mPosition.x)+10.0f, (int)(mPosition.y), 0.0f));
    
    updateLabel();
}

void Quake::trigger(const float duration)
{
    mState = STATE_TRIGGERED;
    
    const float maxGridLineAlpha = 0.45f;
    
    //console() << "---- begin\n";
    if( duration > 0.0f )
    {
        mGridLinesAlpha = 0.0f;
        mMarkerAlpha = 0.0f;
        
        timeline().apply( &mGridLinesAlpha, maxGridLineAlpha, duration*0.25f, EaseOutExpo() );
        timeline().appendTo( &mGridLinesAlpha, maxGridLineAlpha, 0.0f, duration*0.5f, EaseInCubic() );
        
        timeline().apply( &mMarkerAlpha, 1.0f, 0.0f, duration, EaseInCubic() )
        .finishFn( std::bind( &Quake::endTrigger, this ) );
        timeline().apply( &mMarkerSize, 0.0f, 1.0f, duration, EaseOutCubic() );
    }
    else
    {
        mGridLinesAlpha = 0.1f;
        mMarkerAlpha = 0.65f;
        mMarkerSize = 0.5f;
    }
    
}

void Quake::endTrigger()
{
    //console() << "---- end\n";
    mState = STATE_IDLE;
}

void Quake::update(double dt)
{
    if( mState != STATE_TRIGGERED )
    {
        return;
    }
    
    //console() << "mGridLinesAlpha = " << mGridLinesAlpha << ", mMarkerAlpha = " << mMarkerAlpha << std::endl;
    //updateLabel();
}

void Quake::draw()
{
    assert(false && "use the custom draw");
}

void Quake::draw(const ci::Color& markerColor, const float ringsMultiplier, const float radiusMultiplier )
{
    if( mState != STATE_TRIGGERED )
    {
        return;
    }
    
    //gl::disableDepthRead();
    
    // lines
    const float screenWidth = mParentScene->getApp()->getViewportWidth();
    const float screenHeight = mParentScene->getApp()->getViewportHeight();
    //const float width = 1.0f;
    
    gl::color( 1.0f, 1.0f, 1.0f, mGridLinesAlpha );
    gl::drawLine( Vec2f(mPosition.x, 0.0f), Vec2f(mPosition.x, screenHeight) );
    gl::drawLine( Vec2f(0.0f, mPosition.y), Vec2f(screenWidth, mPosition.y) );
    /*
     glBegin( GL_QUADS );
     glVertex2f( mPosition.x, 0.0f );
     glVertex2f( mPosition.x + width, 0.0f );
     glVertex2f( mPosition.x + width, screenHeight );
     glVertex2f( mPosition.x, screenHeight );
     glEnd();
     glBegin( GL_QUADS );
     glVertex2f( mPosition.x, 0.0f );
     glVertex2f( mPosition.x + width, 0.0f );
     glVertex2f( mPosition.x + width, screenHeight );
     glVertex2f( mPosition.x, screenHeight );
     glEnd();
     */
    
    // marker
    const int num_circles = std::max( 2, (int)((int)(mEventData->getMag())*ringsMultiplier) );
    const float spacing = 5.0f;
    const float colorRamp = 0.075f;
    for( int i = 0; i < num_circles; ++i )
    {
        gl::color(markerColor.r, markerColor.g, markerColor.b, mMarkerAlpha - i*colorRamp);
        float maxRadius = std::max( 3.0f + i*spacing, (mEventData->getMag()*radiusMultiplier + i*spacing) );
        float radius = mMarkerSize * maxRadius;
        if( i == 0 )
        {
            gl::drawSolidCircle(Vec2f(mPosition.x, mPosition.y), radius);
        }
        else
        {
            gl::drawStrokedCircle(Vec2f(mPosition.x, mPosition.y), radius);
        }
    }
    
    //drawLabel();
}

void Quake::updateLabel()
{
    if( mShowLabel )
    {
        char buf[256];
        snprintf(buf,256,"%.1f\n%.1f, %.1f", 
                 //mEventData->getTitle().c_str(), 
                 mEventData->getMag(), 
                 //mEventData->getDepth(),
                 mEventData->getLat(), mEventData->getLong());
        //snprintf(buf,256,"%.1f m/s\n%.3f\n%.3f", mVelocity.length(), mRadiusMultiplier, mPeakRadiusMultiplier);
        //snprintf(buf,256,"%s\n%.1f m/s\n%.4e km", mName.c_str(), mVelocity.length(), (mPosition.length()/1000.f));
        mLabel.setText(buf);
    }
}

void Quake::drawLabel()
{
    if( mState != STATE_TRIGGERED )
    {
        return;
    }
    
    if( mShowLabel )
    {
        //Vec2f textCoords = mParentScene->getCamera().worldToScreen(screenCoords, width, height);
        const float minAlpha = 0.45f;
        mLabel.setTextColor(ColorA(1.0f,1.0f,1.0f,mGridLinesAlpha+minAlpha));
        mLabel.draw();
    }
}
