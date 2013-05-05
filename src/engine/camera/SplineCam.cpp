//
//  SplineCam.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2012-12-30.
//
//

#include "SplineCam.h"
#include "Interface.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include <vector>

using namespace ci;
using namespace std;

SplineCam::SplineCam()
{
}

SplineCam::~SplineCam()
{
}

void SplineCam::setup(const float maxDistance, const float radius)
{
    mTarget = Vec3f::zero();
    mSpeed = 0.25f;
    mRadius = radius;
    mLookForward = false;
    mMaxDistance = maxDistance;
    
    //mCam.setFarClip(10000.0f);
    //mCam.setNearClip(0.0001f);
    mCam.setPerspective(60.0f, 1280.0f/768.0f, 10000.0f, 0.0001f);
    
    resetSpline();
}

bool SplineCam::resetSpline()
{
    vector<Vec3f> points;
	int numPoints = 4 + ( Rand::randInt(4) );
	for( int p = 0; p < numPoints; ++p )
    {
		points.push_back( Vec3f( Rand::randFloat(-mRadius/2.0f, mRadius/2.0f), Rand::randFloat(-mRadius/2.0f, mRadius/2.0f), Rand::randFloat(-mRadius/2.0f, mRadius/2.0f) ) );
    }
	mSpline = BSpline3f( points, 3, true, false );
    
	mSplineValue = 0.0f;
	mLastPos = mSpline.getPosition( 0 );
    
    return false;
}

void SplineCam::setupInterface( Interface* interface, const std::string& name)
{
    //interface->gui()->addColumn();
    interface->gui()->addLabel("Spline Cam");
    interface->addParam(CreateBoolParam( "Look Fwd", &mLookForward )
                        .oscReceiver(name, "lookfwd"));
    interface->addParam(CreateFloatParam( "Radius", &mRadius )
                         .minValue(1.0f)
                         .maxValue(mMaxDistance)
                         .oscReceiver(name, "splinecamradius"));
    interface->addButton(CreateTriggerParam("Reset Spline", NULL)
                          .oscReceiver(name,"splinecamreset"))->registerCallback( this, &SplineCam::resetSpline );
    interface->addParam(CreateFloatParam( "Speed", &mSpeed )
                         .minValue(0.0f)
                         .maxValue(1.0f)
                         .oscReceiver(name, "splinecamspeed"));
    interface->addParam(CreateFloatParam( "Speed Multi", &mSpeedMulti )
                        .minValue(0.0f)
                        .maxValue(5.0f)
                        .oscReceiver(name, "splinecamspeedmult"));
}

void SplineCam::update(double dt)
{
    mSplineValue += dt * (mSpeed*0.01f);
    Vec3f pos = mSpline.getPosition( mSplineValue );
    Vec3f delta = pos - mLastPos;
    Vec3f up = delta.cross(pos);
    //up.normalize();
    if( mLookForward )
    {
        mCam.setEyePoint( pos );
        mCam.setViewDirection(delta);
    }
    else
    {
        mCam.lookAt( pos, mTarget, up );
    }
    mLastPos = pos;
}

void SplineCam::drawSpline()
{
    gl::pushMatrices();
    gl::setMatrices( mCam );
	const int numSegments = 100;
	gl::color( ColorA( 0.2f, 0.85f, 0.8f, 0.85f ) );
	glLineWidth( 2.0f );
	gl::begin( GL_LINE_STRIP );
	for( int s = 0; s <= numSegments; ++s )
    {
		float t = s / (float)numSegments;
		gl::vertex( mSpline.getPosition( t ) );
	}
	gl::end();
    gl::popMatrices();
}