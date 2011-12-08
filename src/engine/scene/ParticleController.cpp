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
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "cinder/Perlin.h"
#include <list>

using namespace ci;

ParticleController::ParticleController()
: mPerlin(3)// octaves
, mCounter(0)
{
    mEnabledForces[FORCE_PERLIN] = false;
    mEnabledForces[FORCE_GRAVITY] = false;
    mEnabledForces[FORCE_REPULSION] = true;
}

void ParticleController::setup()
{
    mCounter = 0;
}

void ParticleController::update(double dt)
{
	
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
    glColor4f( 1, 1, 1, 1 );
	glBegin( GL_QUADS );
	for( ParticleList::iterator particleIt = mParticles.begin(); particleIt != mParticles.end(); ++particleIt ) 
    {
        particleIt->draw();
	}
	glEnd();
    
    //trails
    glDisable( GL_TEXTURE_2D );
    for( ParticleList::iterator particleIt = mParticles.begin(); particleIt != mParticles.end(); ++particleIt ) 
    {
		particleIt->drawTrail();
	}
    
    mCounter++;
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
        float lifespan = 0.0f;//Rand::randFloat( 5.0f, 70.0f );
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

// to make walls
//void ParticleController::createConstraints( Vec2f windowDim )
//{
//	mConstraints.clear();
//	//mConstraints.push_back( new Constraint( Vec3f( 1, 0, 0 ), 0.0f, windowDim.x ) );
//	mConstraints.push_back( new Constraint( Vec3f( 0, 1, 0 ), -1000.0f, windowDim.y * 0.625f ) );
//}