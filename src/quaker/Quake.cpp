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

Quake::Quake(Scene* scene, const QuakeEvent* data)
: Entity<float>(scene,Vec3f::zero())
, mLabel(scene)
, mEventData(data)
, mIsLabelVisible(false)
{
    mLabel.setPosition(Vec3f(10.0f, 0.0f, 0.0f));
    mLabel.setFont("Menlo", 10.0f);
    mLabel.setTextColor( ColorA(1.0f,1.0f,1.0f,0.95f) );
    
    // map location
    Vec2f mapPos = Utils::toEquirectProjection( data->getLat(), data->getLong(), scene->getApp()->getViewportWidth(), scene->getApp()->getViewportHeight() );
    mPosition.x = mapPos.x;
    mPosition.y = mapPos.y;
    mPosition.z = 0.0f;
}

Quake::~Quake()
{
}

void Quake::setup()
{
}

void Quake::update(double dt)
{
    updateLabel();
}

void Quake::draw()
{
    glColor4f(1.0f, 0.0f, 0.0f, 0.75f);
    gl::drawSolidCircle(Vec2f(mPosition.x, mPosition.y), mEventData->getMag());
    
    // label
    if( mIsLabelVisible )
    {
        drawLabel();
    }
}

void Quake::updateLabel()
{
    if( mIsLabelVisible )
    {
        char buf[256];
        snprintf(buf,256,"%.1f, %.1f", mEventData->getLat(), mEventData->getLong());
        //snprintf(buf,256,"%.1f m/s\n%.3f\n%.3f", mVelocity.length(), mRadiusMultiplier, mPeakRadiusMultiplier);
        //snprintf(buf,256,"%s\n%.1f m/s\n%.4e km", mName.c_str(), mVelocity.length(), (mPosition.length()/1000.f));
        mLabel.setText(buf);
    }
}

void Quake::drawLabel()
{
    gl::pushMatrices();
    
    const float width = mParentScene->getApp()->getViewportWidth();
    const float height = mParentScene->getApp()->getViewportHeight();
    
    CameraOrtho textCam(0.0f, width, height, 0.0f, 0.0f, 10.f);
    gl::setMatrices(textCam);
    
    //Vec2f textCoords = mParentScene->getCamera().worldToScreen(screenCoords, width, height);
    glTranslatef(mPosition.x, mPosition.y, 0.0f);
    
    mLabel.draw();
    gl::popMatrices();
}
