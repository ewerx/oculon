/*
 *  Body.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "cinder/gl/gl.h"
#include "cinder/App/App.h"
#include "cinder/CinderMath.h"
#include "cinder/Rand.h"
#include "cinder/Camera.h"
#include "Body.h"
#include "OculonApp.h"

using namespace ci;

//
// Body
// 

Body::Body(const Vec3d& pos, const Vec3d& vel, float radius, double mass, const ColorA& color)
: Entity<double>(pos)
, mVelocity(vel)
, mRadius(radius)
, mRadiusMultiplier(1.0f)
, mMass(mass)
, mColor(color)
, mIsLabelVisible(true)
{
    mLabel.setPosition(Vec3d(10.0f, 0.0f, 0.0f));
    mLabel.setFont("Synchro LET");
}

Body::~Body()
{
}

void Body::setup()
{
}

void Body::update(double dt)
{
    mPosition += mVelocity * dt;
    
    updateLabel();
}

void Body::draw(const Matrix44d& transform)
{
    static const int sphereDetail = 64;
    //static const int minTrailLength = 64;
    //static const double scale = 6e-12 * 1.f;
    Vec3d screenCoords = transform * mPosition;
    const int trailLength = 128 + 128 * randFloat();//(mMotionTrail.getPoints().front() - mMotionTrail.getPoints().back())//minTrailLength + minTrailLength*(math<double>::abs(mPosition.length())*scale);
    //app::console() << "trail length: " << trailLength << std::endl;
    
     
    glPushMatrix();
    {
        //glEnable( GL_LIGHTING );
        
        glTranslatef(screenCoords.x, screenCoords.y, screenCoords.z);
        
        glMaterialfv( GL_FRONT, GL_DIFFUSE,	mColor );
        //glColor4f( mColor.r, mColor.g, mColor.b, mColor.a );
        gl::drawSphere( Vec3d::zero(), mRadius*mRadiusMultiplier, sphereDetail );
        
        // label
        if( mIsLabelVisible )
        {
            gl::pushMatrices();
        
            CameraOrtho textCam(0.0f, app::getWindowWidth(), app::getWindowHeight(), 0.0f, 0.0f, 10.f);
            gl::setMatrices(textCam);
        
            OculonApp* app = static_cast<OculonApp*>(App::get());
            Vec2f textCoords = app->getCamera().worldToScreen(screenCoords, app::getWindowWidth(), app::getWindowHeight());
            glTranslatef(textCoords.x, textCoords.y, 0.0f);

            mLabel.draw();

            gl::popMatrices();
        }
         
    }
    glPopMatrix();
    
    glPushMatrix();
    {
        if( mMotionTrail.size() > trailLength )
        {
            mMotionTrail.getPoints().erase(mMotionTrail.begin());
        }
        mMotionTrail.push_back( Vec3f(screenCoords.x, screenCoords.y, screenCoords.z) );
        gl::draw(mMotionTrail);
    }
    glPopMatrix();
}

void Body::applyForceFromBody(Body& otherBody, double dt, double gravConst)
{
    if( &otherBody != this )
    {
        Vec3d dir = this->mPosition - otherBody.mPosition;
        double distSqrd = dir.lengthSquared();
        if( distSqrd > 0 )
        {
            dir.normalize();
            double accel = -gravConst * ( otherBody.mMass / distSqrd );
            mVelocity += dir * accel * dt;
        }
    }
}

void Body::updateLabel()
{
    if( mIsLabelVisible )
    {
        char buf[256];
        snprintf(buf,256,"v = %.1f m/s", mVelocity.length());
        mLabel.setText(buf);
    }
}
