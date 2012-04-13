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
#include "Orbiter.h"

#include "Binned.h" //TODO: hack

using namespace ci;
using namespace std;

GLfloat Body::no_mat[]			= { 0.0f, 0.0, 0.0f, 1.0f };
GLfloat Body::mat_ambient[]		= { 0.5f, 0.5, 0.5f, 1.0f };
GLfloat Body::mat_diffuse[]		= { 0.8f, 0.8, 0.8f, 1.0f };
GLfloat Body::mat_specular[]	= { 1.0f, 1.0, 1.0f, 1.0f };
GLfloat Body::mat_emission[]	= { 0.2f, 0.2, 0.2f, 0.0f };

GLfloat Body::mat_shininess[]	= { 128.0f };
GLfloat Body::no_shininess[]	= { 0.0f };

//
// Body
// 

Body::Body(Scene* scene, string name, const Vec3d& pos, const Vec3d& vel, float radius, double rotSpeed, double mass, const ColorA& color)
: Entity<double>(scene,pos)
, mName(name)
, mVelocity(vel)
, mAcceleration(0.0f)
, mRotationSpeed(rotSpeed)
, mRadius(radius)
, mRadiusMultiplier(1.0f)
, mPeakRadiusMultiplier(1.0f)
, mMass(mass)
, mColor(color)
, mHasTexture(false)
, mIsLabelVisible(true)
, mLabel(scene)
{
    mEaseFactor = 1.0f;
    mLabel.setPosition(Vec3d(10.0f, 0.0f, 0.0f));
    mLabel.setFont("Menlo", 10.0f);
    mLabel.setTextColor( ColorA(1.0f,1.0f,1.0f,0.95f) );
}

Body::Body(Scene* scene, string name, const Vec3d& pos, const Vec3d& vel, float radius, double rotSpeed, double mass, const ColorA& color, ImageSourceRef textureImage)
: Entity<double>(scene,pos)
, mName(name)
, mVelocity(vel)
, mAcceleration(0.0f)
, mRotationSpeed(rotSpeed)
, mRadius(radius)
, mRadiusMultiplier(1.0f)
, mPeakRadiusMultiplier(1.0f)
, mMass(mass)
, mColor(color)
, mTexture(textureImage)
, mHasTexture(true)
, mIsLabelVisible(true)
, mLabel(scene)
{
    mEaseFactor = 1.0f;
    mLabel.setPosition(Vec3d(10.0f, 0.0f, 0.0f));
    mLabel.setFont("Menlo", 10.0f);
    mLabel.setTextColor( ColorA(1.0f,1.0f,1.0f,0.95f) );
    
    mTexture.setWrap( GL_REPEAT, GL_REPEAT );
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
    mRotation += toDegrees(mRotationSpeed) * dt * (mRadiusMultiplier*10.f);
    
    updateLabel();
    
    if( mRadiusMultiplier > 1.0f && mRadiusAnimTime < 3.0f)
    {
        mRadiusAnimTime += (float)(mParentScene->getApp()->getElapsedSecondsThisFrame());
        mEaseFactor = easeOutQuad(mRadiusAnimTime);
        mRadiusMultiplier = 1.0f + mEaseFactor * (mPeakRadiusMultiplier - 1.0f);
        //mRadiusMultiplier = math<float>::max( 1.0f, mRadiusMultiplier - dt*mRadiusAnimRate );
    }
}

void Body::draw(const Matrix44d& transform, bool drawBody)
{
    //TODO: make hierarchy Entity <-- Sphere <-- Body, so Sphere can check its own culling
    static const int sphereDetail = 64;
    //fconst float radius = mRadius; //* 0.75f;
    //static const int minTrailLength = 64;
    //static const double scale = 6e-12 * 1.f;
    Vec3d screenCoords = transform * mPosition;
    double distanceFactor = mPosition.length() / 108000000000.f;
    const int trailLength = Orbiter::sMinTrailLength + Orbiter::sMinTrailLength*(int)(distanceFactor) + (int)(mRadius*2);
    
    if( drawBody )
    {
        glPushMatrix();
        //glEnable( GL_LIGHTING );
        glEnable( GL_POLYGON_SMOOTH );
        
        glTranslated(screenCoords.x, screenCoords.y, screenCoords.z);
        glRotated(mRotation, 0.0f, 1.0f, 0.0f);
        
        //drawDebugVectors();
        
        //glMaterialfv( GL_FRONT, GL_AMBIENT,	Body::mat_ambient );
        glMaterialfv( GL_FRONT, GL_AMBIENT,	Body::no_mat );
        glMaterialfv( GL_FRONT, GL_SPECULAR, Body::no_mat );
        glMaterialfv( GL_FRONT, GL_SHININESS, Body::no_shininess );
        glMaterialfv( GL_FRONT, GL_EMISSION, Body::mat_emission );
        
        glMaterialfv( GL_FRONT, GL_DIFFUSE,	ColorA(Orbiter::sPlanetGrayScale, Orbiter::sPlanetGrayScale, Orbiter::sPlanetGrayScale) );
        //glMaterialfv( GL_FRONT, GL_DIFFUSE, mColor );
        bool drawShell = false;
        float radius = drawShell ? mRadius : mRadius * mRadiusMultiplier;
        
        if( mHasTexture ) mTexture.enableAndBind();
        gl::drawSphere( Vec3d::zero(), radius, sphereDetail );
        if( mHasTexture ) mTexture.disable();
        
        if( drawShell )
        {
            glMaterialfv( GL_FRONT, GL_DIFFUSE,	ColorA(Orbiter::sPlanetGrayScale, Orbiter::sPlanetGrayScale, Orbiter::sPlanetGrayScale, 0.5f) );
            gl::drawSphere( Vec3d::zero(), mRadius * mRadiusMultiplier, sphereDetail );
        }
        
        // label
        if( mIsLabelVisible )
        {
            gl::pushMatrices();
        
            CameraOrtho textCam(0.0f, app::getWindowWidth(), app::getWindowHeight(), 0.0f, 0.0f, 10.f);
            gl::setMatrices(textCam);
        
            Vec2f textCoords = mParentScene->getCamera().worldToScreen(screenCoords, app::getWindowWidth(), app::getWindowHeight());
            glTranslatef(textCoords.x, textCoords.y, 0.0f);

            mLabel.draw();
            
            const bool binned = true;
            if( binned )
            {
                //TODO: hack, use a message
                Binned* binnedScene = NULL;//static_cast<Binned*>(app->getScene(1));
                
                if( binnedScene && binnedScene->isRunning() )
                {
                    Vec3d screenCoords = transform * mPosition;
                    //Vec2f textCoords = app->getCamera().worldToScreen(screenCoords, app::getWindowWidth(), app::getWindowHeight());
                    float force = 200.f;
                    binnedScene->addRepulsionForce(textCoords, mRadius*mRadiusMultiplier*0.35f, force*mRadiusMultiplier);
                }
            }

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
            for( int i = 0; i < mMotionTrail.size() - trailLength; ++i )
                mMotionTrail.getPoints().erase(mMotionTrail.begin());
        }
        const float lineShade = 1.0f;
        glColor4f( lineShade, lineShade, lineShade, 0.75f );
        //float audioOffset = (mRadiusMultiplier*10.f); 
        Vec3f trailPoint = screenCoords;//screenCoords + Vec3f( audioOffset*Rand::, audioOffset, audioOffset );
        //console() << "audioOffset = " << audioOffset << std::endl;
        mMotionTrail.push_back( trailPoint );
        
        if( Orbiter::sUseSmoothLines )
        {
            glPushMatrix();
            //glEnable( GL_MULTISAMPLE_ARB );
            //glEnable(GL_BLEND); 
            glEnable( GL_LINE_SMOOTH );
            glEnable( GL_POLYGON_SMOOTH );
            //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//did nothing?
            glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );//did nothing?
            glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );//did nothing?
            glLineWidth(Orbiter::sTrailWidth);
            if( Orbiter::sUseTriStripLine )
            {
                glBegin(GL_TRIANGLE_STRIP);
                const Vec3f& point = mMotionTrail.getPoints()[0];
                glVertex3f(point.x,point.y,point.z);
                for( int i = 1; i < mMotionTrail.size()-3; ++i )
                {
                    // this makes a ribbon...
                    //const Vec3f& point = mMotionTrail.getPoints()[i];//(*it);
                    const Vec3f& point2 = mMotionTrail.getPoints()[i+1];
                    //const Vec3f& point3 = mMotionTrail.getPoints()[i+2];
                    glVertex3f(point2.x,point2.y,point2.z);
                    glVertex3f(point2.x,point2.y+Orbiter::sTrailWidth,point2.z);
                    //glVertex3f(point3.x,point3.y,point3z);
                    //glVertex3f(point3.x+4,point3.y,point3z);
                }
            }
            else 
            {
                glLineWidth(Orbiter::sTrailWidth);
                //gl::draw(mMotionTrail);
                glBegin( GL_LINE_STRIP );
                for( PolyLine<Vec3f>::iterator it = mMotionTrail.begin(); it != mMotionTrail.end(); ++it )
                {
                    const Vec3f& point = (*it);
                    glVertex3f(point.x,point.y,point.z);
                }
                glLineWidth(1.0f);
            }
            glEnd();
            //glDisable(GL_MULTISAMPLE_ARB );
            //glDisable(GL_BLEND); 
            glDisable(GL_LINE_SMOOTH );
            glDisable(GL_POLYGON_SMOOTH );
            glPopMatrix();
        }
        else
        {
            gl::draw(mMotionTrail);
        }
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
            mAcceleration = -gravConst * ( otherBody.mMass / distSqrd );
            mVelocity += dir * mAcceleration * dt;
        }
    }
}

void Body::updateLabel()
{
    if( mIsLabelVisible )
    {
        char buf[256];
        snprintf(buf,256,"%.4f AU\n%.1f m/s", mPosition.length()/149598000000.0f, mVelocity.length());
        //snprintf(buf,256,"%.1f m/s\n%.3f\n%.3f", mVelocity.length(), mRadiusMultiplier, mPeakRadiusMultiplier);
        //snprintf(buf,256,"%s\n%.1f m/s\n%.4e km", mName.c_str(), mVelocity.length(), (mPosition.length()/1000.f));
        mLabel.setText(buf);
    }
}

void Body::resetTrail()
{
    mMotionTrail.getPoints().clear();
}

void Body::applyFftBandValue( float fftBandValue )
{
    mRadiusAnimTime += (float)(mParentScene->getApp()->getElapsedSecondsThisFrame());
    
    int framesToAvgFft = Orbiter::sNumFramesToAvgFft;
    if( mLastFftValues.size() > framesToAvgFft )
    {
        mLastFftValues.erase(mLastFftValues.begin());
    }
    mLastFftValues.push_back(fftBandValue);
    
    float avgFftValue = 0.0f;
    for (vector<float>::iterator it = mLastFftValues.begin(); it != mLastFftValues.end(); ++it) 
    {
        avgFftValue += (*it);
    }
    avgFftValue /= mLastFftValues.size();
    
    float mult = 1.0f + avgFftValue * Orbiter::sMaxRadiusMultiplier;
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

