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
{
}

Binned::~Binned()
{
}

void Binned::setup()
{
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
	int binPower = 4;
	
    // this clears the particle list
	mParticleSystem.setup(getWindowWidth(), getWindowHeight(), binPower);
	
	mKParticles = 12;
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
	
	mTimeStep = 1;
	mLineOpacity = 0.12f;
	mPointOpacity = 0.5f;
	mSlowMotion = false;
	mParticleNeighborhood = 14;
	mParticleRepulsion = 1.5;
	mCenterAttraction = .05;
}

void Binned::resize()
{
    reset();
}

void Binned::update(double /*dt*/)
{
	mParticleSystem.setTimeStep(mTimeStep);
    
    // debug info
    char buf[256];
    snprintf(buf, 256, "particles: %d", mKParticles * 1024);
    mApp->getInfoPanel().addLine(buf, Color(0.75f, 0.5f, 0.5f));
}

void Binned::draw()
{
    gl::pushMatrices();
	gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
    
    gl::disableDepthRead();
    //gl::disableDepthWrite();
    glDisable(GL_LIGHTING);
	gl::enableAdditiveBlending();
	glColor4f(1.0f, 1.0f, 1.0f, mLineOpacity);
	
	mParticleSystem.setupForces();
	// apply per-particle forces
	glBegin(GL_LINES);
	for(int i = 0; i < mParticleSystem.size(); i++) 
    {
		Particle& cur = mParticleSystem[i];
		// global force on other particles
		mParticleSystem.addRepulsionForce(cur, mParticleNeighborhood, mParticleRepulsion);
		// forces on this particle
		cur.bounceOffWalls(0, 0, getWindowWidth(), getWindowHeight());
		cur.addDampingForce();
	}
	glEnd();
	// single global forces
	mParticleSystem.addAttractionForce(getWindowWidth()/2, getWindowHeight()/2, getWindowWidth(), mCenterAttraction);
	if(mIsMousePressed)
    {
		mParticleSystem.addRepulsionForce(mMousePos.x, mMousePos.y, 100, 10);
    }
	mParticleSystem.update();
	glColor4f(1.0f, 1.0f, 1.0f, mPointOpacity);
	mParticleSystem.draw();
    
    gl::enableDepthRead();
    gl::enableAlphaBlending();
    gl::popMatrices();
}


bool Binned::handleKeyDown( const KeyEvent& event )
{
    bool handled = false;
    
	if( event.getChar() == 's' ) 
    {
		mSlowMotion = !mSlowMotion;
		if(mSlowMotion)
        {
			mTimeStep = .05;
        }
		else
        {
			mTimeStep = 1;
        }
        
        handled = true;
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

