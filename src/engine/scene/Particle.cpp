/*
 *  Particle.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-11-23.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Particle.h"
#include "cinder/Rand.h"
#include "cinder/Perlin.h"
#include "cinder/gl/gl.h"
#include <boost/circular_buffer.hpp>

using namespace ci;

Particle::Particle()
: Entity<float>(Vec3f::zero())
, mVelocity(Vec3f::zero())
, mAccel(Vec3f::zero())
, mRadius(0.0f)
, mMass(0.0f)
, mInvMass(0.0f)
, mCharge(0.0f)
, mAge(0.0f)
, mLifeSpan(0.0f)
, mPosHistory(TRAIL_LENGTH)
, mState(STATE_NORMAL)
{
    reset();
}

Particle::Particle(const Vec3f& pos, const Vec3f& vel, float radius, float mass, float charge, float lifespan)
: Entity<float>(pos)
, mVelocity(vel)
, mAccel(Vec3f::zero())
, mRadius(radius)
, mMass(mass)
, mCharge(charge)
, mAge(0.0f)
, mLifeSpan(lifespan)
, mPosHistory(TRAIL_LENGTH)
, mState(STATE_NORMAL)
{
    reset(pos,vel,radius,mass,charge,lifespan);
}

Particle::~Particle()
{
}

void Particle::reset()
{
    reset(Vec3f::zero(), Vec3f::zero(), 1.0f, 1.0f, 0.0f, 1.0f);
}

void Particle::reset(const Vec3f& pos, const Vec3f& vel, float radius, float mass, float charge, float lifespan)
{
    mState = STATE_NORMAL;
    
    mPosition = pos;
    mVelocity = vel;
    mAccel = Vec3f::zero();
    mRadius = radius;
    mMass = mass;
    mCharge = charge;
    mAge = 0.0f;
    mLifeSpan = lifespan;
    mPosHistory.clear();
    
    if( mMass > 0.0f )
    {
        mInvMass = 1.0f / mMass;
    }
}

void Particle::update(double dt)
{
    if( mVelocity != Vec3f::zero() )
    {
        mVelocity += mAccel;
    }
    
    //TODO: *dt ?
    mPosition += mVelocity;
    
    mPosHistory.push_back( mPosition );
    
    if( mLifeSpan > 0.0f )
    {
        mAge++;
        if( mAge > mLifeSpan )
        {
            mState = STATE_DEAD;
        }
        else 
        {
            mAgePer = 1.0f - mAge/mLifeSpan;
        }
    }
    else
    {
        mAge++;
        mAgePer = 1.0f;
    }

    // decay velocity
    mVelocity *= 0.975f;
    // reset accel for this frame
    mAccel.set(0.0f,0.0f,0.0f);
}

void Particle::draw(bool useBillboard, const Vec3f& bbRight, const Vec3f &bbUp)
{
    {
    
        //TODO: need this?
        float heat = 0.0f;
        
        if( mState == STATE_DECAY && mVelocity == Vec3f::zero() )
        {
            glColor4f( mAgePer * 0.5f, mAgePer * 0.35f, 1.0f - mAgePer, mAgePer + Rand::randFloat( 0.25f ) );
        }
        else 
        {
            glColor4f( mAgePer, mAgePer * 0.75f, 1.0f - mAgePer + 0.15f, mAgePer + Rand::randFloat( 0.5f ) );
        }
        
        float radius = mRadius;
        if( mAge == 1 && mState != STATE_DECAY && heat > 0.3f )
        {
            //radius *= 20.0f * heat;
        }
        
        if( useBillboard )
        {
            gl::drawBillboard(mPosition, Vec2f(radius,radius), 0.0f, bbRight, bbUp);
        }
        else
        {
            glTexCoord2f( 0, 0 );
            glVertex3f( mPosition.x - radius, mPosition.y - radius, mPosition.z );
            
            glTexCoord2f( 1, 0 );
            glVertex3f( mPosition.x + radius, mPosition.y - radius, mPosition.z );
            
            glTexCoord2f( 1, 1 );
            glVertex3f( mPosition.x + radius, mPosition.y + radius, mPosition.z );
            
            glTexCoord2f( 0, 1 );
            glVertex3f( mPosition.x - radius, mPosition.y + radius, mPosition.z );
        }
    }

}

void Particle::drawTrail()
{
    //renderLineStripTrail();
    renderQuadStripTrail();
}

void Particle::renderLineStripTrail()
{
	int len = math<int>::min( mAge, mPosHistory.size() );
    float inv = 1.0f / (float)len;
    
	glBegin( GL_LINE_STRIP );
    
	for( int i = 0; i < len; i++ )
    {
		float per = 1.0f - ( inv * (float)i );
        
		glColor4f( per, 0.2f, 0.0f, per * mAgePer * 0.2f );
		gl::vertex( mPosHistory[i] );
	}
    
	glEnd();
}


void Particle::renderQuadStripTrail()
{
    int size = mPosHistory.size();
    int len = math<int>::min( mAge, mPosHistory.size() );
    
	glBegin( GL_QUAD_STRIP );
    
	for( int i = 0; i < (size - 2); i++ )
    {
        float per = i / (float)(len-1);
		Vec3f perp0	= Vec3f( mPosHistory[i].x, mPosHistory[i].y, 0.0f ) - Vec3f( mPosHistory[i+1].x, mPosHistory[i+1].y, 0.0f );
		Vec3f perp1	= perp0.cross( Vec3f::zAxis() );
		Vec3f perp2	= perp0.cross( perp1 );
        perp1	= perp0.cross( perp2 ).normalized();
        
		Vec3f off	= perp1 * ( mRadius * ( 1.0f - per ) * 0.25f  );
        
		glColor4f( ( 1.0f - per ) * 0.75f, 0.15f, per * 0.5f, ( 1.0f - per ) * mAgePer * 0.25f );
		gl::vertex( mPosHistory[i] - off );
		gl::vertex( mPosHistory[i] + off );
	}
    
	glEnd();
}

void Particle::applyGravity(double dt)
{
}

void Particle::applyPerlin(Perlin& perlin, int counter, double dt)
{
    static float magnitude = 1.0f;
    
    Vec3f noise = perlin.dfBm( Vec3f(  mPosition.x, 
                                        mPosition.y, 
                                        mPosition.z ) * 0.01f + Vec3f( 0, 0, counter / 100.0f ) );
    noise.normalize();
    noise *= magnitude;
    mAccel += Vec3f( noise.x, noise.y, noise.z );
}

void Particle::applyRepulsion(Particle& other, double dt)
{
    static float magnitude = 1.0f;
    
    Vec3f dir = this->mPosition - other.mPosition;
    float distSqrd = dir.lengthSquared();
    static float radiusSum = 125.0f;//( p1->mRadius + p2->mRadius ) * 100.0f;
    static float radiusSqrd = radiusSum * radiusSum;
    static float thisQTimesInvM = mInvMass * mCharge;
    
    if( distSqrd < radiusSqrd && distSqrd > 0.1f ) 
    {
        float per = 1.0f - distSqrd/radiusSqrd;
        float E = other.getCharge() / distSqrd;
        float F = E * thisQTimesInvM;
        
        if( F > 15.0f )
        {
            F = 15.0f;
        }
        
        dir.normalize();
        dir *= F * per * magnitude;
        
        this->mAccel += dir;
        other.mAccel -= dir;
    }
}
