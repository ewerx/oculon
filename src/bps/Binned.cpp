/*
 *  Binned.cpp
 *  Oculon
 *
 *  Created by Ehsan on 12-01-30.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Binned.h"
#include "BinnedParticleSystem.h"

#include "cinder/app/AppBasic.h"
#include "cinder/Utilities.h"
//#include "cinder/gl/gl.h" <-- included in Particle.h
#include "cinder/Rand.h"
#include "OculonApp.h"

using namespace ci;
using namespace ci::app;
using namespace bps;

Binned::Binned()
: mMode(MODE_BOTH)
, mTopBottom(false)
, mForceScaleX(1.0f)
, mForceScaleY(1.0f)
, mRandomPlacement(true)
{
}

Binned::~Binned()
{
}

void Binned::setup()
{
    mKParticles = 16;
    
    mTimeStep = 1;
	//mLineOpacity = 0.12f;
	//mPointOpacity = 0.5f;
	mSlowMotion = false;
	mParticleNeighborhood = 14;
    
	mParticleRepulsion = 1.5;
	mCenterAttraction = 0.05;
    
    mDamping = 0.01f;
    mWallDamping = 0.3f;
    
    mMinForce = 0.0f;
    mMinRadius = 0.0f;
    mMaxForce = 150.0f;
    mMaxRadius = mApp->getWindowHeight() * 0.3f;
    mAudioSensitivity = 1.0f;
    
    mPointColor.r = 1.0f;
    mPointColor.g = 1.0f;
    mPointColor.b = 1.0f;
    mPointColor.a = 0.5f;
    
    mForceColor.r = 1.0f;
    mForceColor.g = 0.0f;
    mForceColor.b = 0.0f;
    mForceColor.a = 0.12f;
    
    mParticleSystem.setForceColor( &mForceColor );
    
    reset();
}

void Binned::reset()
{
    // this number describes how many bins are used
	// on my machine, 2 is the ideal number (2^2 = 4x4 pixel bins)
	// if this number is too high, binning is not effective
	// because the screen is not subdivided enough. if
	// it's too low, the bins take up so much memory as to
	// become inefficient.
	int binPower = 2;
	
    // this clears the particle list
	mParticleSystem.setup(getWindowWidth(), getWindowHeight(), binPower);
	
	float padding = 0;
	float maxVelocity = .5;
	for(int i = 0; i < mKParticles * 1024; i++) 
    {
		float x = Rand::randFloat(padding, getWindowWidth() - padding);
		float y = Rand::randFloat(padding, getWindowHeight() - padding);
		float xv = Rand::randFloat(-maxVelocity, maxVelocity);
		float yv = Rand::randFloat(-maxVelocity, maxVelocity);
		Particle particle(x, y, xv, yv);
		mParticleSystem.add(particle);
	}
    
    while( !mQueuedForces.empty() )
        mQueuedForces.pop();
}

void Binned::resize()
{
    reset();
}

void Binned::setupParams(params::InterfaceGl& params)
{
    params.addText( "binned", "label=`Binned`" );
    params.addParam("Mode", &mMode, "");
    params.addParam("Slow Motion", &mSlowMotion, "");
    params.addParam("Time Step", &mTimeStep, "step=0.01 min=0.01 max=1.0");
    params.addParam("Random Placement", &mRandomPlacement, "");
    params.addParam("Top/Bottom", &mTopBottom, "");
    params.addParam("Particle Repulsion", &mParticleRepulsion, "step=0.01");
    params.addParam("Damping Force", &mDamping, "step=0.01");
    params.addParam("Wall Damping", &mWallDamping, "step=0.01");
    params.addParam("Center Attraction", &mCenterAttraction, "step=0.01");
    params.addParam("Force Scale X", &mForceScaleX, "step=0.1");
    params.addParam("Force Scale Y", &mForceScaleY, "step=0.1");
    params.addParam("Min Force", &mMinForce, "");
    params.addParam("Max Force", &mMaxForce, "");
    params.addParam("Min Radius", &mMinRadius, "");
    params.addParam("Max Radius", &mMaxRadius, "");
    params.addParam("Audio Sensitivity", &mAudioSensitivity, "step=0.01 min=0.0");
    params.addParam("K Particles", &mKParticles, "min=1 max=100");
    params.addParam("Point Color", &mPointColor, "");
    params.addParam("Force Color", &mForceColor, "");
}

void Binned::update(double /*dt*/)
{
	mParticleSystem.setTimeStep(mTimeStep);
    
    // debug info
    char buf[256];
    snprintf(buf, 256, "particles: %d", mKParticles * 1024);
    mApp->getInfoPanel().addLine(buf, Color(0.75f, 0.5f, 0.5f));
    
    snprintf(buf, 256, "mode: %d", mMode);
    mApp->getInfoPanel().addLine(buf, Color(0.75f, 0.75f, 0.75f));
}

void Binned::draw()
{
    gl::pushMatrices();
	gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
    
    gl::disableDepthRead();
    //gl::disableDepthWrite();
    glDisable(GL_LIGHTING);
	gl::enableAdditiveBlending();
	//glColor4f(1.0f, 1.0f, 1.0f, mLineOpacity);
	
	mParticleSystem.setupForces();
	// apply per-particle forces
	glBegin(GL_LINES);
	for(int i = 0; i < mParticleSystem.size(); i++) 
    {
		Particle& cur = mParticleSystem[i];
		// global force on other particles
		mParticleSystem.addRepulsionForce(cur, mParticleNeighborhood, mParticleRepulsion);
		// forces on this particle
		cur.bounceOffWalls(0, 0, getWindowWidth(), getWindowHeight(), mWallDamping);
		cur.addDampingForce(mDamping);
	}
	glEnd();
	// single global forces
	mParticleSystem.addAttractionForce(getWindowWidth()/2, getWindowHeight()/2, getWindowWidth(), mCenterAttraction);
	if(mIsMousePressed)
    {
        const float radius = mMaxRadius*0.5f;
        const float force = mMaxForce*0.5f;
        mParticleSystem.addRepulsionForce(mMousePos.x, mMousePos.y, radius, force*mForceScaleX, force*mForceScaleY);
    }
    
    const bool orbiter_mode = false; 
    if( orbiter_mode )
    {
        applyQueuedForces();
    }
    else
    {
        updateAudioResponse();
    }
    
	mParticleSystem.update();
	//glColor4f(1.0f, 1.0f, 1.0f, mPointOpacity);
	mParticleSystem.draw( mPointColor );
    
    gl::enableDepthRead();
    gl::enableAlphaBlending();
    gl::popMatrices();
}

void Binned::updateAudioResponse()
{
    // audio response
    AudioInput& audioInput = mApp->getAudioInput();
    std::shared_ptr<float> fftDataRef = audioInput.getFftDataRef();
    
    unsigned int bandCount = audioInput.getFftBandCount();
    float* fftBuffer = fftDataRef.get();
    
    int spacing = mApp->getWindowWidth() / bandCount;
    spacing *=2;
    int x1 = mApp->getWindowWidth() / 2;
    int x2 = x1;
    //int numBandsPerBody = 1;
    
    if( fftBuffer )
    {
        for( int i = 0; i < bandCount; i += 2) 
        {
            float avgFft = fftBuffer[i] / bandCount;
            //avgFft /= (float)(numBandsPerBody);
            avgFft *= mAudioSensitivity;
            
            x1 += spacing;
            x2 -= spacing;
            
            float radius = mMinRadius;
            float force = mMinForce;
            
            switch( mMode )
            {
                case MODE_RADIUS:
                    radius += mMaxRadius * avgFft;
                    force = mMaxForce / 2.0f;
                    break;
                case MODE_FORCE:
                    radius = mMinRadius * 5.0f;
                    force += mMaxForce * avgFft;
                    break;
                case MODE_BOTH:
                    radius += mMaxRadius * avgFft;
                    force += mMaxForce * avgFft;
                    break;
                default:
                    assert(false && "invalid mode");
                    break;
            }
            
            if( mRandomPlacement )
            {
                x1 = Rand::randFloat(0,getWindowWidth());
                x2 = Rand::randFloat(0,getWindowWidth());
            }
            const float magX = force * mForceScaleX;
            const float magY = force * mForceScaleY;
            const float centerY = mRandomPlacement ? Rand::randFloat(0,getWindowHeight()) :(getWindowHeight()/2.0f);
            
            if( mTopBottom )
            {
                mParticleSystem.addRepulsionForce(x1, 0, radius, magX, magY);
                mParticleSystem.addRepulsionForce(x2, 0, radius, magX, magY);
            
                mParticleSystem.addRepulsionForce(x1, getWindowHeight(), radius, magX, magY);
                mParticleSystem.addRepulsionForce(x2, getWindowHeight(), radius, magX, magY);
            }
            else
            {
                mParticleSystem.addRepulsionForce(x1, centerY, radius, magX, magY);
                mParticleSystem.addRepulsionForce(x2, centerY, radius, magX, magY);
            }
            
            bool debug = false;
            if( debug )
            {
                const float maxRadius = 50.0f;
                
                glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
                gl::drawSolidCircle(Vec2f(x1, centerY), maxRadius * avgFft);
                gl::drawSolidCircle(Vec2f(x2, centerY), maxRadius * avgFft);
            }
        }
    }
}

bool Binned::handleKeyDown( const KeyEvent& event )
{
    bool handled = true;
    
	switch( event.getChar() )
    {
        case 's':
            mSlowMotion = !mSlowMotion;
            if(mSlowMotion)
            {
                mTimeStep = .05;
            }
            else
            {
                mTimeStep = 1;
            }
            break;
            
        case 'T':
            mTopBottom = !mTopBottom;
            break;
            
        case 'm':
        {
            int mode = mMode;
            mode++;
            if( mode >= MODE_COUNT )
                mode = 0;
            mMode = static_cast<eMode>(mode);
            break;
        }
            
        case ' ':
            reset();
            break;
            
        default:
            handled = false;
            break;
    }
    
    return handled;
}

void Binned::handleMouseDown( const MouseEvent& event )
{
	mIsMousePressed = true;
	mMousePos = Vec2i(event.getPos());
}

void Binned::handleMouseUp( const MouseEvent& event )
{
	mIsMousePressed = false;
}

void Binned::handleMouseDrag( const MouseEvent& event )
{
	mMousePos = Vec2i(event.getPos());
}

void Binned::addRepulsionForce( const Vec2f& pos, float radius, float force )
{
    tRepulsionForce repulsion;
    repulsion.mPos = pos;
    repulsion.mRadius = radius;
    repulsion.mForce = force;
    mQueuedForces.push( repulsion );
}

void Binned::applyQueuedForces()
{
    while( ! mQueuedForces.empty() )
    {
        tRepulsionForce& repulsion = mQueuedForces.front();
        mParticleSystem.addRepulsionForce(repulsion.mPos.x, repulsion.mPos.y, repulsion.mRadius, repulsion.mForce*mForceScaleX, repulsion.mForce*mForceScaleY);
        mQueuedForces.pop();
    }
}
