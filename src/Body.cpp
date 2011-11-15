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
#include "cinder/Easing.h"
#include "Body.h"
#include "OculonApp.h"

using namespace ci;

GLfloat Body::no_mat[]			= { 0.0, 0.0, 0.0, 1.0 };
GLfloat Body::mat_ambient[]		= { 0.5, 0.5, 0.5, 1.0 };
GLfloat Body::mat_diffuse[]		= { 0.8, 0.8, 0.8, 1.0 };
GLfloat Body::mat_specular[]	= { 1.0, 1.0, 1.0, 1.0 };
GLfloat Body::mat_emission[]	= { 0.1, 0.1, 0.1, 0.0 };

GLfloat Body::mat_shininess[]	= { 128.0 };
GLfloat Body::no_shininess[]	= { 0.0 };

//
// Body
// 

Body::Body(const Vec3d& pos, const Vec3d& vel, float radius, double mass, const ColorA& color)
: Entity<double>(pos)
, mVelocity(vel)
, mRadius(radius)
, mRadiusMultiplier(1.0f)
, mPeakRadiusMultiplier(1.0f)
, mMass(mass)
, mColor(color)
, mIsLabelVisible(true)
{
    mLabel.setPosition(Vec3d(10.0f, 0.0f, 0.0f));
    mLabel.setFont("Monaco", 9.0f);
    mLabel.setTextColor( ColorA(0.9f,0.9f,0.9f,0.9f) );
}

Body::~Body()
{
}

void Body::setup()
{
    mRadiusAnimRate = 0.000005f;
}

void Body::update(double dt)
{
    mPosition += mVelocity * dt;
    
    updateLabel();
    
    if( mRadiusMultiplier > 1.0f && mRadiusAnimTime < 2.0f)
    {
        //TODO: fix this hack
        OculonApp* oculon = static_cast<OculonApp*>(App::get());
        mRadiusAnimTime += oculon->getElapsedSecondsThisFrame();
        float easeFactor = easeOutQuad(mRadiusAnimTime);
        mRadiusMultiplier =  easeFactor * mPeakRadiusMultiplier;
        //mRadiusMultiplier = math<float>::max( 1.0f, mRadiusMultiplier - dt*mRadiusAnimRate );
    }
}

void Body::draw(const Matrix44d& transform, bool drawBody)
{
    //TODO: make hierarchy Entity <-- Sphere <-- Body, so Sphere can check its own culling
    static const int sphereDetail = 64;
    const float radius = mRadius * mRadiusMultiplier * 0.75f;
    //static const int minTrailLength = 64;
    //static const double scale = 6e-12 * 1.f;
    Vec3d screenCoords = transform * mPosition;
    const int trailLength = 128 + (int)(screenCoords.length()/10.f) + (int)(radius);
    
    if( drawBody )
    {
        glPushMatrix();
        //glEnable( GL_LIGHTING );
        
        glTranslatef(screenCoords.x, screenCoords.y, screenCoords.z);
        
        //drawDebugVectors();
        
        //glMaterialfv( GL_FRONT, GL_AMBIENT,	Body::mat_ambient );
        glMaterialfv( GL_FRONT, GL_AMBIENT,	Body::no_mat );
        glMaterialfv( GL_FRONT, GL_SPECULAR, Body::no_mat );
        glMaterialfv( GL_FRONT, GL_SHININESS, Body::no_shininess );
        glMaterialfv( GL_FRONT, GL_EMISSION, Body::mat_emission );
        
        glMaterialfv( GL_FRONT, GL_DIFFUSE,	mColor );
        //glColor4f( mColor.r, mColor.g, mColor.b, mColor.a );
        gl::drawSphere( Vec3d::zero(), radius, sphereDetail );
        
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
        glPopMatrix();
    }
    
    glDisable(GL_LIGHTING);
    // always draw trail
    glPushMatrix();
    {
        if( mMotionTrail.size() > trailLength )
        {
            mMotionTrail.getPoints().erase(mMotionTrail.begin());
        }
        glColor4f( 0.5f, 0.5f, 0.5f, 0.5f );
        //float audioOffset = (mRadiusMultiplier*10.f); 
        Vec3f trailPoint = screenCoords;//screenCoords + Vec3f( audioOffset*Rand::, audioOffset, audioOffset );
        //console() << "audioOffset = " << audioOffset << std::endl;
        mMotionTrail.push_back( trailPoint );
        gl::draw(mMotionTrail);
    }
    glPopMatrix();
    glEnable(GL_LIGHTING);
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
        //snprintf(buf,256,"%.1f m/s\n%.2f km", mVelocity.length(), mPosition.length()/1000.f);
        snprintf(buf,256,"%.1f m/s\n%.3f\n%.3f", mVelocity.length(), mRadiusMultiplier, mPeakRadiusMultiplier);
        mLabel.setText(buf);
    }
}

void Body::resetTrail()
{
    mMotionTrail.getPoints().clear();
}

void Body::applyFftBandValue( float fftBandValue )
{
    float mult = 1.0f + fftBandValue * 4.0f; // max 4x radius
    if( mult > mRadiusMultiplier )
    {
        mPeakRadiusMultiplier = mult;
        mRadiusMultiplier = mult;
        mRadiusAnimTime = 0.0f;
    }
}

void Body::drawDebugVectors()
{
    float radius = mRadiusMultiplier*mRadius*2.0f;
    glDisable(GL_LIGHTING);
    glColor4f( 1.0f, 0.0f, 0.0f, 1.0f );
    gl::drawVector( Vec3f::zero(), Vec3f::xAxis()*radius*2.0f );
    glColor4f( 0.0f, 1.0f, 0.0f, 1.0f );
    gl::drawVector( Vec3f::zero(), Vec3f::yAxis()*radius*2.0f );
    glColor4f( 0.0f, 0.0f, 1.0f, 1.0f );
    gl::drawVector( Vec3f::zero(), Vec3f::zAxis()*radius*2.0f );
    glEnable(GL_LIGHTING);
}

