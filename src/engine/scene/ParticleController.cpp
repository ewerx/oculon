/*
 *  ParticleController.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-11-26.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "ParticleController.h"
#include "Particle.h"
#include "Resources.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "cinder/Perlin.h"
#include "cinder/App/App.h"
#include "cinder/ImageIo.h"
#include <list>

using namespace ci;

ParticleController::ParticleController()
: mPerlin(3)// octaves
, mCounter(0)
, mDrawAsSpheres(true)
{
    mEnabledForces[FORCE_PERLIN] = false;
    mEnabledForces[FORCE_GRAVITY] = true;
    mEnabledForces[FORCE_REPULSION] = false;
}

void ParticleController::setup()
{
    mCounter = 0;
    
    mParticleTexture = gl::Texture( loadImage( app::loadResource( RES_PARTICLE ) ) );
    mParticleTexture.setWrap( GL_REPEAT, GL_REPEAT );
}

void ParticleController::update(double dt)
{
	static bool newway = false;
    if (newway )
    {
        applyForces2(dt);
    }
    
    for( ParticleList::iterator particleIt = mParticles.begin(); particleIt != mParticles.end(); ++particleIt ) 
    {
		if( ! particleIt->isState(Particle::STATE_DEAD) ) 
        {
			
//			if( particleIt->mIsBouncing ){
//				if( Rand::randFloat() < 0.025f && !particleIt->mIsDying ){
//					mParticles.push_back( Particle( particleIt->mLoc[0], Vec3f::zero() ) );
//					mParticles.back().mIsDying = true;
//					particleIt->mIsDying = true;
//					//particleIt->mVel += Rand::randVec3f() * Rand::randFloat( 2.0f, 3.0f );
//				}
//			}
            
            if( !newway )
                applyForces( particleIt, dt );
            
			particleIt->update(dt);
		}
		else 
        {
			particleIt = mParticles.erase( particleIt );
		}
	}
	
	// apply Constraints;
//	for( vector<Constraint*>::iterator constraintIt = mConstraints.begin(); constraintIt != mConstraints.end(); ++constraintIt ){
//		if( mEnableConstraints ){
//			(*constraintIt)->apply( mParticles );
//			(*constraintIt)->apply( emitter );
//		}
//	}
}

void ParticleController::draw()
{
    glEnable( GL_TEXTURE_2D );
    gl::enableAdditiveBlending();
    gl::enableDepthWrite( false );
    
    mParticleTexture.bind();
    glColor4f( 1, 1, 1, 1 );
    
    if( !mDrawAsSpheres )
    {
        glBegin( GL_QUADS );
    }
	for( ParticleList::iterator particleIt = mParticles.begin(); particleIt != mParticles.end(); ++particleIt ) 
    {
        if( particleIt->mRadius > 0.1f )
            particleIt->draw(mDrawAsSpheres);
	}
    if( !mDrawAsSpheres )
    {
        glEnd();
    }
    mParticleTexture.unbind();
    
    //trails
    glDisable( GL_TEXTURE_2D );
    for( ParticleList::iterator particleIt = mParticles.begin(); particleIt != mParticles.end(); ++particleIt ) 
    {
		particleIt->drawTrail();
	}
    
    mCounter++;
    
    gl::enableDepthWrite( true );
    gl::enableAlphaBlending();
}

void ParticleController::addParticles( int amt, Vec3f pos, Vec3f vel, float radius )
{
    float heat = 0.0f;
    
    for( int i = 0; i < amt; i++ ) 
    {
		Vec3f lOffset = Rand::randVec3f();
		Vec3f p = pos + lOffset * radius * 0.25f;
		Vec3f v = -vel + lOffset * Rand::randFloat( 6.0f, 10.5f ) * ( heat + 0.75f ) + Rand::randVec3f() * Rand::randFloat( 1.0f, 2.0f );
		v.y *= 0.65f;
        float pRadius = Rand::randFloat( 1.0f, 3.0f );
        float mass = pRadius;
        float charge = Rand::randFloat( 0.35f, 0.75f );
        float lifespan = Rand::randFloat( 5.0f, 70.0f ); // 0.
		mParticles.push_back( Particle( p, v, radius, mass, charge, lifespan ) );
	}
}

void ParticleController::applyForces( ParticleController::ParticleList::iterator p1, double dt )
{
    for( int i = 0; i < FORCE_COUNT; ++i )
    {
        if( mEnabledForces[i] )
        {
            switch (static_cast<eForce>(i)) 
            {
                case FORCE_GRAVITY:
                    p1->applyGravity(dt);
                    break;
                case FORCE_PERLIN:
                    p1->applyPerlin(mPerlin, mCounter, dt);
                    break;
                case FORCE_REPULSION:
                {
                    for( ParticleList::iterator p2 = p1; p2 != mParticles.end(); ++p2 )
                    {
                        if( p2 != p1 )
                        {
                            p1->applyRepulsion((*p2), dt);
                        }
                    }
                }
                    break;
                default:
                    break;
            }
        }
    }
}

void ParticleController::applyForces2( double dt )
{
    static float magnitude = 1.0f;
    
    for( int i = 0; i < FORCE_COUNT; ++i )
    {
        if( mEnabledForces[i] )
        {
            switch (static_cast<eForce>(i)) 
            {
                case FORCE_GRAVITY:
                    for( ParticleList::iterator pIt = mParticles.begin(); pIt != mParticles.end(); ++pIt )
                    {
                        pIt->applyGravity(dt);
                    }
                    break;
                case FORCE_PERLIN:
                    for( ParticleList::iterator pIt = mParticles.begin(); pIt != mParticles.end(); ++pIt )
                    {
                        pIt->applyPerlin(mPerlin, mCounter, dt);
                    }
                    break;
                case FORCE_REPULSION:
                {
                    for( ParticleList::iterator p1 = mParticles.begin(); p1 != mParticles.end(); ++p1 )
                    {
                        float thisQTimesInvM = p1->mInvMass * p1->mCharge;
                        
                        for( ParticleList::iterator p2 = p1; p2 != mParticles.end(); ++p2 )
                        {
                            if( p1 != p2 )
                            {
                                Vec3f dir = p1->getPosition() - p2->getPosition();
                                float distSqrd = dir.lengthSquared();
                                float radiusSum = 125.0f;//( p1->mRadius + p2->mRadius ) * 100.0f;
                                static float radiusSqrd = radiusSum * radiusSum;
                                
                                if( distSqrd < radiusSqrd && distSqrd > 0.1f ) 
                                {
                                    float per = 1.0f - distSqrd/radiusSqrd;
                                    float E = p2->mCharge / distSqrd;
                                    float F = E * thisQTimesInvM;
                                    
                                    if( F > 15.0f )
                                        F = 15.0f;
                                    
                                    dir.normalize();
                                    dir *= F * per * magnitude;
                                    
                                    p1->mAccel += dir;
                                    p2->mAccel -= dir;
                                }
                            }
                        }
                    }
                }
                break;
                default:
                    break;
            }
        }
    }
}

// to make walls
//void ParticleController::createConstraints( Vec2f windowDim )
//{
//	mConstraints.clear();
//	//mConstraints.push_back( new Constraint( Vec3f( 1, 0, 0 ), 0.0f, windowDim.x ) );
//	mConstraints.push_back( new Constraint( Vec3f( 0, 1, 0 ), -1000.0f, windowDim.y * 0.625f ) );
//}