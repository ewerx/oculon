/*
 *  Scene.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-17.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Scene.h"
#include "OculonApp.h" //TODO: this dependency shouldn't exist

Scene::Scene()
: mApp(NULL)
, mIsActive(true)
, mIsVisible(true)
, mIsFrustumPlaneCached(false)
, mEnableFrustumCulling(false)
{
    // frustum culling
	for( int i=0; i<SIDE_COUNT; i++ )
    {
		mCachedFrustumPlane.mNormals[i] = Vec3f::zero();
		mCachedFrustumPlane.mPoints[i] = Vec3f::zero();
		mCachedFrustumPlane.mDists[i] = 0.0f;
	}
}

Scene::~Scene()
{
}

void Scene::update(double dt)
{
    if( mEnableFrustumCulling )
    {
        mIsFrustumPlaneCached = false;
        calcNearAndFarClipCoordinates( mApp->getCamera() ); //TODO: scene should own the camera
    }
}

void Scene::calcNearAndFarClipCoordinates( const Camera &cam )
{
	Vec3f ntl, ntr, nbl, nbr;
	cam.getNearClipCoordinates( &ntl, &ntr, &nbl, &nbr );
    
	Vec3f ftl, ftr, fbl, fbr;
	cam.getFarClipCoordinates( &ftl, &ftr, &fbl, &fbr );
	
	if( ! mIsFrustumPlaneCached )
    {
		calcFrustumPlane( mCachedFrustumPlane.mNormals[TOP], mCachedFrustumPlane.mPoints[TOP], mCachedFrustumPlane.mDists[TOP], ntr, ntl, ftl );
		calcFrustumPlane( mCachedFrustumPlane.mNormals[BOT], mCachedFrustumPlane.mPoints[BOT], mCachedFrustumPlane.mDists[BOT], nbl, nbr, fbr );
		calcFrustumPlane( mCachedFrustumPlane.mNormals[LEF], mCachedFrustumPlane.mPoints[LEF], mCachedFrustumPlane.mDists[LEF], ntl, nbl, fbl );
		calcFrustumPlane( mCachedFrustumPlane.mNormals[RIG], mCachedFrustumPlane.mPoints[RIG], mCachedFrustumPlane.mDists[RIG], ftr, fbr, nbr );
		calcFrustumPlane( mCachedFrustumPlane.mNormals[NEA], mCachedFrustumPlane.mPoints[NEA], mCachedFrustumPlane.mDists[NEA], ntl, ntr, nbr );
		calcFrustumPlane( mCachedFrustumPlane.mNormals[FARP], mCachedFrustumPlane.mPoints[FARP], mCachedFrustumPlane.mDists[FARP], ftr, ftl, fbl );
	}
	
	mIsFrustumPlaneCached = true;
}

// need 'const'?
void Scene::calcFrustumPlane( Vec3f &fNormal, Vec3f &fPoint, float &fDist, const Vec3f &v1, const Vec3f &v2, const Vec3f &v3 )
{
	Vec3f aux1, aux2;
    
	aux1 = v1 - v2;
	aux2 = v3 - v2;
    
	fNormal = aux2.cross( aux1 );
	fNormal.normalize();
	fPoint.set( v2 );
	fDist = -( fNormal.dot( fPoint ) );
}

bool Scene::isPointInFrustum( const Vec3f &loc )
{
	float d;
	bool result = true;
    
	for( int i=0; i<SIDE_COUNT; i++ )
    {
		d = mCachedFrustumPlane.mDists[i] + mCachedFrustumPlane.mNormals[i].dot( loc );
		if( d < 0 )
			return( false );
	}
    
	return( result );
}


bool Scene::isSphereInFrustum( const Vec3f &loc, float radius )
{
	float d;
	bool result = true;
	
	for(int i=0; i<SIDE_COUNT; i++ )
    {
		d = mCachedFrustumPlane.mDists[i] + mCachedFrustumPlane.mNormals[i].dot( loc );
		if( d < -radius )
			return( false );
	}
	
	return( result );
}


bool Scene::isBoxInFrustum( const Vec3f &loc, const Vec3f &size )
{
	float d;
	int out, in;
	bool result = true;
	
	Vec3f vertex[8];
	
	vertex[0] = Vec3f( loc.x + size.x, loc.y + size.y, loc.z + size.z );
	vertex[1] = Vec3f( loc.x + size.x, loc.y - size.y, loc.z + size.z );
	vertex[2] = Vec3f( loc.x + size.x, loc.y - size.y, loc.z - size.z );
	vertex[3] = Vec3f( loc.x + size.x, loc.y + size.y, loc.z - size.z );
	vertex[4] = Vec3f( loc.x - size.x, loc.y + size.y, loc.z + size.z );
	vertex[5] = Vec3f( loc.x - size.x, loc.y - size.y, loc.z + size.z );
	vertex[6] = Vec3f( loc.x - size.x, loc.y - size.y, loc.z - size.z );
	vertex[7] = Vec3f( loc.x - size.x, loc.y + size.y, loc.z - size.z );
	
	for( int i=0; i<SIDE_COUNT; i++ )
    {
		out = 0;
		in = 0;
        
		for( int k=0; k<8 && ( in==0 || out==0 ); k++ )
        {
			d = mCachedFrustumPlane.mDists[i] + mCachedFrustumPlane.mNormals[i].dot( vertex[k] );
			
			if( d < 0 )
				out ++;
			else
				in ++;
		}
        
		if( !in )
			return( false );
	}
	
	return( result );
}
