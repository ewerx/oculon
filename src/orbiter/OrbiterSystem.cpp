//
//  OrbiterSystem.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-10-25.
//
//

#include "Orbiter.h"

using namespace ci;
using namespace std;

#pragma mark - System

Orbiter::System::System(const std::string& name)
: mName(name)
{
    mGravitation = 1.0f;
}

Orbiter::System::~System()
{
    
}

void Orbiter::System::addBody(BodyRef body)
{
    mBodies.push_back(body);
}

void Orbiter::System::update(double dt)
{
    for( BodyRef body : mBodies )
    {
        for( BodyRef otherBody : mBodies )
        {
            if (&otherBody != &body)
            {
                body->applyForceFromBody( *otherBody, dt, mGravitation );
            }
        }
        
        body->update(dt);
    }
}

void Orbiter::System::draw()
{
    for( BodyRef body : mBodies )
    {
        body->draw();
    }
}

#pragma mark - Body

Orbiter::Body::Body(const std::string& name,
                    const ci::Vec3d& pos,
                    const ci::Vec3d& vel,
                    float radius,
                    double rotationSpeed,
                    double mass)
: mName(name)
, mPosition(pos)
, mVelocity(vel)
, mRadius(radius)
, mRotationSpeed(rotationSpeed)
, mMass(mass)
{
    
}

void Orbiter::Body::setTextures(ci::ImageSourceRef diffuse, ci::ImageSourceRef normalMap)
{
    gl::Texture::Format format;
    //format.setColorInternalFormat( GL_RGBA32F_ARB );
    format.setWrap( GL_REPEAT, GL_REPEAT );
    
    mTexDiffuse = gl::Texture( diffuse );
    mTexNormal = gl::Texture( normalMap );
}

void Orbiter::Body::applyForceFromBody(Body &otherBody, double dt, float gravitation)
{
    if( &otherBody != this )
    {
        Vec3f dir = this->mPosition - otherBody.mPosition;
        float distSqrd = dir.lengthSquared();
        if( distSqrd > EPSILON )
        {
            dir.normalize();
            float acceleration = -gravitation * ( otherBody.mMass / distSqrd );
            mVelocity += dir * acceleration * dt;
        }
    }
}

void Orbiter::Body::update(double dt)
{
    mPosition += mVelocity * dt;
    //mRotation += toDegrees(mRotationSpeed) * dt;
}

void Orbiter::Body::draw()
{
    mTexDiffuse.bind( 0 );
    mTexNormal.bind( 1 );
    
    gl::drawSphere( mPosition, mRadius, 64 );
    
    mTexNormal.unbind();
    mTexDiffuse.unbind();
}
