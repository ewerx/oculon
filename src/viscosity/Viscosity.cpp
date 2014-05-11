//
//  Viscosity.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-11.
//
//

#include "OculonApp.h"
#include "Viscosity.h"
#include "cinder/Camera.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace std;

#pragma mark - Construction

Viscosity::Viscosity()
: Scene("viscosity")
{
}

Viscosity::~Viscosity()
{
}

#pragma mark - Setup

void Viscosity::setup()
{
    Scene::setup();

    mCamera.setOrtho( 0, mApp->getViewportWidth(), mApp->getViewportHeight(), 0, -1, 1 );
    
    mFluid2D.set( mApp->getViewportWidth()/4.0f, mApp->getViewportHeight()/4.0f );
   	mFluid2D.setDensityDissipation( 0.99f );
	mFluid2D.setRgbDissipation( 0.99f );
	mFluid2D.enableDensity();
	mFluid2D.enableRgb();
	mFluid2D.enableVorticityConfinement();
	mFluid2D.initSimData();
}

void Viscosity::reset()
{
}

#pragma mark - Interface

void Viscosity::setupInterface()
{

}

void Viscosity::handleMouseDown(const ci::app::MouseEvent &event)
{
    mPrevPos = event.getPos();
}

void Viscosity::handleMouseDrag(const ci::app::MouseEvent &event)
{
    float x = (event.getX() / (float)mApp->getViewportWidth()) * mFluid2D.resX();
	float y = (event.getY() / (float)mApp->getViewportHeight()) * mFluid2D.resY();
	
    float mVelScale = 600.0f;
    float mRgbScale = 50.0f;
    float mDenScale = 50.0f;
    Colorf mColor = Colorf::white();
    
	if( event.isLeftDown() )
    {
		Vec2f dv = event.getPos() - mPrevPos;
		mFluid2D.splatVelocity( x, y, mVelScale*dv );
		mFluid2D.splatRgb( x, y, mRgbScale*mColor );
		if( mFluid2D.isBuoyancyEnabled() )
        {
			mFluid2D.splatDensity( x, y, mDenScale );
		}
	}
	
	mPrevPos = event.getPos();
}

#pragma mark - Update

void Viscosity::update(double dt)
{
    mFluid2D.step();
}

#pragma mark - Draw

const Camera& Viscosity::getCamera()
{
    return mCamera;
}

void Viscosity::draw()
{
    gl::pushMatrices();
    gl::setMatrices( getCamera() );
    
    float* data = const_cast<float*>( (float*)mFluid2D.rgb().data() );
	Surface32f surf( data, mFluid2D.resX(), mFluid2D.resY(), mFluid2D.resX()*sizeof(Colorf), SurfaceChannelOrder::RGB );
	
    
	if ( ! mTex )
    {
		mTex = gl::Texture( surf );
	}
    else
    {
		mTex.update( surf );
	}
	gl::draw( mTex, mApp->getViewportBounds() );
	mTex.unbind();
    
    gl::popMatrices();
}

void Viscosity::drawDebug()
{
    Scene::drawDebug();
    
}
