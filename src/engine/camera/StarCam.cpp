/*
 Copyright (c) 2010-2012, Paul Houx - All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
 the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#include "StarCam.h"
#include "Utils.h"

#include "cinder/CinderMath.h"
#include "cinder/Utilities.h"
#include "cinder/app/AppBasic.h"
#include "Scene.h"
#include "OculonApp.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// initialize static members
const double StarCam::LATITUDE_LIMIT = 89.0;
const double StarCam::LATITUDE_THRESHOLD = 89.0;
const double StarCam::DISTANCE_MIN = 0.015;
const double StarCam::DISTANCE_MAX = 1000.0;

StarCam::StarCam()
: mScene(NULL)
{
	mInitialCam = mCurrentCam = CameraPersp(); 
	mCurrentCam.setNearClip( 5.0f );//0.01f );
	mCurrentCam.setFarClip( 200000.0f );//5000.0f );

	mDeltaX = mDeltaY = mDeltaD = 0.0; 
	mDeltas.clear();
	mIsMouseDown = false;

	mLatitude = 0.0;
	mLongitude = 0.0;
	mDistance = 0.015 * 33333;
	mFov = 65.0;
    
    mTimeScale = NULL;
    mRotateSpeed = NULL;
    mLookAt = Vec3f::zero();
}

void StarCam::setup(Scene* scene)
{
    mScene = scene;
	mTimeOut = 300.0;
	mTimeMouse = getElapsedSeconds() - mTimeOut;
    mElapsedTime = 0.0f;
    mCurrentCam.setPerspective( 65.0f, mScene->getApp()->getViewportAspectRatio(), 5.0f, 200000.0f );
}

void StarCam::update(double elapsed)
{
	static const double sqrt2pi = math<double>::sqrt(2.0 * M_PI);

	if((getElapsedSeconds() - mTimeMouse) > mTimeOut)	// screensaver mode
	{
        mElapsedTime += elapsed * ( mTimeScale ? (*mTimeScale) : 0.005f );
        
		// calculate time 
		double t = getElapsedSeconds();

		// determine distance to the sun (in parsecs)
		double time = mElapsedTime;//t * ( mTimeScale ? (*mTimeScale) : 0.005f );
		double t_frac = (time) - math<double>::floor(time);
		double n = sqrt2pi * t_frac;
		double f = cos( n * n );
		double distance = DISTANCE_MAX - (DISTANCE_MAX-DISTANCE_MIN) * f;//500.0 - 499.95 * f;

		// determine where to look
		double longitude = toDegrees( mElapsedTime );//toDegrees(t * ( mTimeScale ? (*mTimeScale * 2) : 0.034f ));
		double latitude = LATITUDE_LIMIT * sin(t * 0.029);

		// determine interpolation factor
		t = math<double>::clamp( (getElapsedSeconds() - mTimeMouse - mTimeOut) / 100.0, 0.0, 1.0);

		// interpolate over time to where we should be, so that the camera doesn't snap from user mode to screensaver mode
		mDistance = lerp<double>( mDistance.value(), distance, t);
		mLatitude = lerp<double>( mLatitude.value(), latitude, t ); 
		// to prevent rotating from 180 to -180 degrees, always rotate over the shortest distance
		mLongitude = mLongitude.value() + lerp<double>( 0.0, Utils::wrap(longitude - mLongitude.value(), -180.0, 180.0), t );
	}
	else	// user mode
	{		
		// adjust for frame rate
		float t = (float) elapsed / 0.25f;
		if(t > 1.0f) t = 1.0f;

		// zoom in by decreasing distance to 102.5 units
		//mDistance = (1.0f - t) * mDistance.value() + t * 102.5f;

		if(!mIsMouseDown) {
			// update longitude speed and value
			mDeltaX *= 0.975;
			mLongitude = Utils::wrap( mLongitude.value() - mDeltaX, -180.0, 180.0 ); 

			// update latitude speed and value
			mDeltaY *= 0.975;
			mLatitude = math<double>::clamp( mLatitude.value() + mDeltaY, -LATITUDE_LIMIT, LATITUDE_LIMIT );

			// update distance
			//mDeltaD *= 0.975;
			//mDistance = math<double>::clamp( mDistance.value() + mDeltaD, DISTANCE_MIN, DISTANCE_MAX );

			// move latitude back to its threshold
			if( mLatitude.value() < -LATITUDE_THRESHOLD ) {
				mLatitude = 0.9f * mLatitude.value() + 0.1f * -LATITUDE_THRESHOLD;
				mDeltaY = 0.0;
			}
			else if( mLatitude.value() > LATITUDE_THRESHOLD ) {
				mLatitude = 0.9f * mLatitude.value() + 0.1f * LATITUDE_THRESHOLD;
				mDeltaY = 0.0;
			}
		}
		else {
			// add deltas to vector to determine average speed
			mDeltas.push_back( Vec3d(mDeltaX, mDeltaY, mDeltaD) );

			// only keep the last 6 deltas
			while(mDeltas.size() > 6) mDeltas.pop_front();

			// reset deltas
			mDeltaX = 0.0;
			mDeltaY = 0.0;
			mDeltaD = 0.0;
		}

		// adjust field-of-view to 60 degrees
		//mFov = (1.0f - t) * mFov.value() + t * 60.0f;
	}
    
    // update current camera
	mCurrentCam.setFov( mFov.value() );
	mCurrentCam.setEyePoint( getPosition() );
	mCurrentCam.setCenterOfInterestPoint( mLookAt );
}

void StarCam::mouseDown( const Vec2i &mousePos )
{
	mInitialMousePos = mousePos;
	mTimeMouse = getElapsedSeconds();

	mInitialCam = mCurrentCam;

	mDeltaX = mDeltaY = mDeltaD = 0.0;
	mDeltas.clear();

	mIsMouseDown = true;
}

void StarCam::mouseDrag( const Vec2i &mousePos, bool leftDown, bool middleDown, bool rightDown )
{
	double elapsed = getElapsedSeconds() - mTimeMouse;
	if(elapsed < (1.0/60.0)) return;

	double sensitivity = 0.075;

	if(leftDown) {
		// adjust longitude (east-west)
		mDeltaX = (mousePos.x - mInitialMousePos.x) * sensitivity;
		mLongitude = Utils::wrap( mLongitude.value() - mDeltaX, -180.0, 180.0 ); 
		// adjust latitude (north-south)
		mDeltaY = (mousePos.y - mInitialMousePos.y) * sensitivity;
		mLatitude = math<double>::clamp( mLatitude.value() + mDeltaY, -LATITUDE_LIMIT, LATITUDE_LIMIT );
	}
	else if(rightDown) {
		mDeltaD = ( mousePos.x - mInitialMousePos.x ) + ( mousePos.y - mInitialMousePos.y );
		// adjust distance
		sensitivity = math<double>::max(0.005, math<double>::log10(mDistance) / math<double>::log10(100.0) * 0.075);
		mDistance = math<double>::clamp( mDistance.value() + mDeltaD * sensitivity, DISTANCE_MIN, DISTANCE_MAX );
	}

	mInitialMousePos = mousePos;
	mTimeMouse = getElapsedSeconds();
}

void StarCam::mouseUp(  const Vec2i &mousePos  ) 
{	
	mInitialMousePos = mousePos;
	mTimeMouse = getElapsedSeconds();

	mIsMouseDown = false;

	// calculate average delta (speed) over the last 10 frames 
	// and use that to rotate the camera after mouseUp
	Vec3d avg = average(mDeltas);
	mDeltaX = avg.x;
	mDeltaY = avg.y;
	mDeltaD = avg.z;
}

void StarCam::resize()
{
	//mCurrentCam.setAspectRatio(event.getAspectRatio());
}

const CameraPersp& StarCam::getCamera()  
{
	return mCurrentCam;
}

void StarCam::setCurrentCam( const CameraPersp &aCurrentCam ) 
{ 
	mCurrentCam = aCurrentCam;

	// update distance and fov
	mDistance = (mCurrentCam.getEyePoint() - mLookAt).length();
	mFov = mCurrentCam.getFov();
}

Vec3f StarCam::getPosition()
{
	// calculates position based on current distance, longitude and latitude
	double theta = M_PI - toRadians(mLongitude.value());
	double phi = 0.5 * M_PI - toRadians(mLatitude.value());

	Vec3f orientation( 
		static_cast<float>( sin(phi) * cos(theta) ),
		static_cast<float>( cos(phi) ),
		static_cast<float>( sin(phi) * sin(theta) ) );

	return mDistance.value() * orientation; 
}

void StarCam::setTarget( const ci::Vec3f& lookAt )
{
    //TODO: interpolate
    mLookAt = lookAt;
}
