//
//  Controller.cpp
//  Star
//
//  Created by Robert Hodgin on 5/1/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "cinder/Rand.h"
#include "CoronaController.h"

const int MAX_DUSTS = 20000;

using namespace ci;
using std::vector;

CoronaController::CoronaController()
{
	mPrevTotalDustVerts	= -1;
	mTotalDustVerts		= 0;
    mDustVerts			= NULL;
}

void CoronaController::update( float dt )
{
	// GLOWS
	for( vector<CoronaGlow>::iterator it = mGlows.begin(); it != mGlows.end(); ){
		if( it->mIsDead ){
			it = mGlows.erase( it );
		} else {
			it->update( dt );
			++ it;
		}
	}
	
	// NEBULAS
	for( vector<CoronaNebula>::iterator it = mNebulas.begin(); it != mNebulas.end(); ){
		if( it->mIsDead ){
			it = mNebulas.erase( it );
		} else {
			it->update( dt );
			++ it;
		}
	}
	
	updateDusts( dt );
}

void CoronaController::updateDusts( float dt )
{
	for( vector<Dust>::iterator it = mDusts.begin(); it != mDusts.end(); ){
		if( it->mIsDead ){
			it = mDusts.erase( it );
		} else {
			it->update( dt );
			it++;
		}
	}
	
	mTotalDustVerts = mDusts.size();
	
    if (mTotalDustVerts != mPrevTotalDustVerts) {
        if (mDustVerts != NULL) {
            delete[] mDustVerts;
        }
        mDustVerts = new DustVertex[mTotalDustVerts];
        mPrevTotalDustVerts = mTotalDustVerts;
    }
	
	int vIndex = 0;
	for( vector<Dust>::iterator it = mDusts.begin(); it != mDusts.end(); ++it ){        
		mDustVerts[vIndex].vertex = it->mPos;
		mDustVerts[vIndex].color  = Vec4f( it->mCol.r, it->mCol.g, it->mCol.b, it->mCol.a );
        vIndex++;
	}
}

void CoronaController::drawGlows( gl::GlslProg *shader, const Vec3f &right, const Vec3f &up )
{
	for( vector<CoronaGlow>::iterator it = mGlows.begin(); it != mGlows.end(); ++it ){
		shader->uniform( "alpha", it->mAgePer );
		it->draw( right, up );
	}
}

void CoronaController::drawNebulas( gl::GlslProg *shader, const Vec3f &right, const Vec3f &up )
{
	for( vector<CoronaNebula>::iterator it = mNebulas.begin(); it != mNebulas.end(); ++it ){
		shader->uniform( "alpha", it->mAgePer );
		it->draw( right, up );
	}
}

void CoronaController::drawDusts()
{
	if( mTotalDustVerts > 0 ){
		glEnableClientState( GL_VERTEX_ARRAY );
		glEnableClientState( GL_COLOR_ARRAY );
		glVertexPointer( 3, GL_FLOAT, sizeof(DustVertex), mDustVerts );
		glColorPointer( 4, GL_FLOAT, sizeof(DustVertex), &mDustVerts[0].color );
		
		glDrawArrays( GL_POINTS, 0, mTotalDustVerts );
		
		glDisableClientState( GL_VERTEX_ARRAY );
		glDisableClientState( GL_COLOR_ARRAY );
	}
	gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
}

void CoronaController::addGlows( const CoronaStar &star, float power, int amt )
{
	for( int i=0; i<amt; i++ ){
		float radius	= Rand::randFloat( 15.0f, 20.0f ) * star.mRadiusMulti;
		Vec3f dir		= Rand::randVec3f();
		Vec3f pos		= star.mPos + dir * ( star.mRadius - radius * 0.25 );
		Vec3f vel		= dir * Rand::randFloat( 1.3f, 2.0f );
		
		float lifespan	= Rand::randFloat( 15.0f, 30.0f ) * star.mRadiusMulti;
		
		mGlows.push_back( CoronaGlow( pos, vel, radius + ( 1.0f - power ) * 35.0f, lifespan ) );
	}
}

void CoronaController::addNebulas( const CoronaStar &star, int amt )
{
	for( int i=0; i<amt; i++ ){
		float radius		= Rand::randFloat( 15.0f, 25.0f ) * star.mRadiusMulti;
		Vec3f dir			= Rand::randVec3f();
		Vec3f pos			= star.mPos + dir * ( star.mRadius - radius * 0.25f );
		Vec3f vel			= dir * Rand::randFloat( 0.2f, 1.0f ) * star.mRadiusMulti;
//		if( Rand::randFloat() < 0.05f ) vel = dir * Rand::randFloat( 1.2f, 1.75f ) * star.mRadiusMulti;
//		float radius		= star.mRadius * Rand::randFloat( 2.25f, 3.0f );
		float lifespan		= Rand::randFloat( 35.0f, 55.0f ) * star.mRadiusMulti;
		mNebulas.push_back( CoronaNebula( pos, vel, radius, lifespan ) );
	}
}

void CoronaController::addDusts( const CoronaStar &star, int amt )
{
	int dustsToMake = amt;
	if( mDusts.size() + amt > MAX_DUSTS ){
		dustsToMake = MAX_DUSTS - ( mDusts.size() + amt );
	}
	
	if( dustsToMake > 0 ){
		for( int i=0; i<dustsToMake; i++ ){
			Vec3f dir   = Rand::randVec3f();
			Vec3f pos	= star.mPos + dir * star.mRadius;
			Vec3f vel	= star.mVel + Rand::randVec3f() + dir * Rand::randFloat( 0.85f, 1.5f );
			mDusts.push_back( Dust( pos, vel ) );
		}
	}
}
		
void CoronaController::addCMNebulas( const Vec3f &cmPos, float cmRadius, float radiusMulti, int amt )
{
	for( int i=0; i<amt; i++ ){
		float angle		= Rand::randFloat( -0.25f, 0.25f );
		float cosA		= cos( angle );
		float sinA		= sin( angle );
		Vec3f dir		= Vec3f( cosA, sinA, 0.0f );
		Vec3f pos		= -cmPos + dir * cmRadius;
		
		Vec3f vel		= dir * Rand::randFloat( 1.2f, 3.0f );
		float radius	= Rand::randFloat( 35.0f, 50.0f ) * radiusMulti;
		float lifespan	= Rand::randFloat( 35.0f, 55.0f );
		mNebulas.push_back( CoronaNebula( pos, vel, radius, lifespan ) );
	}
}

void CoronaController::addCMGlows( const Vec3f &cmPos, float cmRadius, int amt )
{
	for( int i=0; i<amt; i++ ){
		float angle		= Rand::randFloat( -0.25f, 0.25f );
		float cosA		= cos( angle );
		float sinA		= sin( angle );
		Vec3f dir		= Vec3f( cosA, sinA, 0.0f );
		Vec3f pos		= -cmPos + dir * cmRadius;
		
		Vec3f vel		= dir * Rand::randFloat( 4.5f, 8.0f );
		float radius	= Rand::randFloat( 75.0f, 150.0f ) * 2.0f;
		float lifespan	= Rand::randFloat( 35.0f, 55.0f );
		mGlows.push_back( CoronaGlow( pos, vel, radius, lifespan ) );
	}
}
