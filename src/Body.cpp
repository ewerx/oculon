/*
 *  Body.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "cinder/gl/gl.h"

#include "Body.h"

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
{
    mLabel.setPosition(Vec3d(mRadius, mRadius, 0.0f));
    //mMotionTrail.setClosed(true);
}

Body::~Body()
{
}

void Body::update(double dt)
{
    mPosition += mVelocity * dt;
    
    char buf[256];
    snprintf(buf,256,"v = %.1f m/s", mVelocity.length());
    mLabel.setText(buf);
}

void Body::draw(const Matrix44d& transform)
{
    static const int sphereDetail = 64;
    static const int trailLength = 256;
    Vec3d screen_coords = transform * mPosition;
    
    glPushMatrix();
    {
        //glEnable( GL_LIGHTING );
        
        glTranslatef(screen_coords.x, screen_coords.y, screen_coords.z);
        
        //glMaterialfv( GL_FRONT, GL_DIFFUSE,	mColor );
        glColor4f( mColor.r, mColor.g, mColor.b, mColor.a );
        gl::drawSphere( Vec3d::zero(), mRadius*mRadiusMultiplier, sphereDetail );
        
        mLabel.draw();
    }
    glPopMatrix();
    
    glPushMatrix();
    {
        if( mMotionTrail.size() > trailLength )
        {
            //mMotionTrail.getPoints().erase(mMotionTrail.begin());
        }
        mMotionTrail.push_back( Vec3f(screen_coords.x, screen_coords.y, screen_coords.z) );
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
