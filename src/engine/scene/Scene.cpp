/*
 *  Scene.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-17.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Scene.h"
#include "OculonApp.h"
#include "Interface.h"

Scene::Scene()
: mApp(NULL)
, mIsActive(false)
, mIsVisible(false)
, mIsFrustumPlaneCached(false)
, mEnableFrustumCulling(false)
{
    // frustum culling
	for( int i=0; i<SIDE_COUNT; i++ )
    {
		mCachedFrustumPlane[i].mNormal = Vec3f::zero();
		mCachedFrustumPlane[i].mPoint = Vec3f::zero();
		mCachedFrustumPlane[i].mDistance = 0.0f;
	}
}

Scene::~Scene()
{
    delete mInterface;
}

void Scene::init(OculonApp* app)
{
    mApp = app;
    mInterface = new Interface( mApp, &mApp->getOscServer() );
    setup();
    setupParams(app->getParams());
}

void Scene::update(double dt)
{
    if( mEnableFrustumCulling )
    {
        mIsFrustumPlaneCached = false;
        calcNearAndFarClipCoordinates( getCamera() );
    }
    
    assert(mInterface);
    mInterface->update();
}

void Scene::drawInterface()
{
    mInterface->draw();
}

#pragma MARK: Frustum Culling
void Scene::calcNearAndFarClipCoordinates( const Camera &cam )
{
	Vec3f ntl, ntr, nbl, nbr;
	cam.getNearClipCoordinates( &ntl, &ntr, &nbl, &nbr );
    
	Vec3f ftl, ftr, fbl, fbr;
	cam.getFarClipCoordinates( &ftl, &ftr, &fbl, &fbr );
	
	if( ! mIsFrustumPlaneCached )
    {
		calcFrustumPlane( mCachedFrustumPlane[TOP].mNormal, mCachedFrustumPlane[TOP].mPoint, mCachedFrustumPlane[TOP].mDistance, ntr, ntl, ftl );
		calcFrustumPlane( mCachedFrustumPlane[BOT].mNormal, mCachedFrustumPlane[BOT].mPoint, mCachedFrustumPlane[BOT].mDistance, nbl, nbr, fbr );
		calcFrustumPlane( mCachedFrustumPlane[LEF].mNormal, mCachedFrustumPlane[LEF].mPoint, mCachedFrustumPlane[LEF].mDistance, ntl, nbl, fbl );
		calcFrustumPlane( mCachedFrustumPlane[RIG].mNormal, mCachedFrustumPlane[RIG].mPoint, mCachedFrustumPlane[RIG].mDistance, ftr, fbr, nbr );
		calcFrustumPlane( mCachedFrustumPlane[NEA].mNormal, mCachedFrustumPlane[NEA].mPoint, mCachedFrustumPlane[NEA].mDistance, ntl, ntr, nbr );
		calcFrustumPlane( mCachedFrustumPlane[FARP].mNormal, mCachedFrustumPlane[FARP].mPoint, mCachedFrustumPlane[FARP].mDistance, ftr, ftl, fbl );
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
		d = mCachedFrustumPlane[i].mDistance + mCachedFrustumPlane[i].mNormal.dot( loc );
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
		d = mCachedFrustumPlane[i].mDistance + mCachedFrustumPlane[i].mNormal.dot( loc );
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
			d = mCachedFrustumPlane[i].mDistance + mCachedFrustumPlane[i].mNormal.dot( vertex[k] );
			
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

const Camera& Scene::getCamera() const
{
    return mApp->getMayaCam();
}
