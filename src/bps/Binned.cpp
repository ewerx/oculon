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
    mKParticles = 12;
    
    mParticleRadius = 5.0f;
    
    mTimeStep = 0.01f;// 0.05
	mSlowMotion = false;
    mBounceOffWalls = false;
	mParticleNeighborhood = 14;
    
	mParticleRepulsion = 1.5;
	mCenterAttraction = 0.0f;//0.05;
    
    mDamping = 0.01f;
    mWallDamping = 0.3f;
    
    mMinForce = 0.0f;
    mMinRadius = 0.0f;
    mMaxForce = 500.0f;
    mMaxRadius = 540.0f;
    mAudioSensitivity = 0.0f;
    
    mIsMousePressed = false;
    mApplyForcePattern = PATTERN_NONE;
    
    mPointColor.r = 1.0f;
    mPointColor.g = 1.0f;
    mPointColor.b = 1.0f;
    mPointColor.a = 0.5f;
    
    mForceColor.r = 0.85f;
    mForceColor.g = 0.85f;
    mForceColor.b = 0.85f;
    mForceColor.a = 0.12f;
    
    mParticleSystem.setForceColor( &mForceColor );
    
    reset();
}

void Binned::reset()
{
    mFrameCounter = 0; 
    
    // this number describes how many bins are used
	// on my machine, 2 is the ideal number (2^2 = 4x4 pixel bins)
	// if this number is too high, binning is not effective
	// because the screen is not subdivided enough. if
	// it's too low, the bins take up so much memory as to
	// become inefficient.
	int binPower = 2;
	
    // this clears the particle list
	mParticleSystem.setup(getWindowWidth(), getWindowHeight(), binPower, this);
	
	float padding = 0;
	float maxVelocity = .5;
    
    bool offScreen = false;
    if( offScreen )
    {
        
        padding = 250;
        /*
        for(int i = 0; i < mKParticles * 256; i++) 
        {
            float xv = Rand::randFloat(-maxVelocity, maxVelocity);
            float yv = Rand::randFloat(-maxVelocity, maxVelocity);
            
            // left
            float x = Rand::randFloat(-padding*2, -padding);
            float y = Rand::randFloat(padding, getWindowHeight() - padding);
            mParticleSystem.add(Particle(x, y, xv, yv));
            
            // right
            x = Rand::randFloat(getWindowWidth()+padding, getWindowWidth()+padding*2);
            y = Rand::randFloat(-padding, getWindowHeight() + padding);
            mParticleSystem.add(Particle(x, y, xv, yv));
            
            // top
            x = Rand::randFloat(-padding, getWindowWidth() + padding);
            y = Rand::randFloat(-padding*2, -padding);
            mParticleSystem.add(Particle(x, y, xv, yv));
            
            // bottom
            x = Rand::randFloat(-padding, getWindowWidth() + padding);
            y = Rand::randFloat(getWindowHeight()+padding, getWindowHeight()+padding*2);
            mParticleSystem.add(Particle(x, y, xv, yv));
        }
         */
        const float centerX = getWindowWidth()/2.0f;
        const float centerY = getWindowHeight()/2.0f;
        const float thickness = 50;
        for(int i = 0; i < mKParticles * 1024; i++) 
        {
            float r = Rand::randFloat(0.0f,M_PI*2.0f);
            float x = centerX + sin(r)*(centerX-padding) + Rand::randFloat(-thickness,thickness);
            float y = centerY + cos(r)*(centerX-padding) + Rand::randFloat(-thickness,thickness);
            float xv = Rand::randFloat(-maxVelocity, maxVelocity);
            float yv = Rand::randFloat(-maxVelocity, maxVelocity);
            mParticleSystem.add(Particle(x, y, xv, yv));
        }
    }
    else
    {
        for(int i = 0; i < mKParticles * 1024; i++) 
        {
            float x = Rand::randFloat(padding, getWindowWidth() - padding);
            float y = Rand::randFloat(padding, getWindowHeight() - padding);
            float xv = Rand::randFloat(-maxVelocity, maxVelocity);
            float yv = Rand::randFloat(-maxVelocity, maxVelocity);
            Particle particle(x, y, xv, yv);
            mParticleSystem.add(particle);
        }
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
    params.addParam("Time Step", &mTimeStep, "step=0.001 min=0.0001 max=1.0");
    params.addParam("Wall Bounce", &mBounceOffWalls, "");
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
    params.addParam("Particle Radius", &mParticleRadius, "min=1 max=50");
}

void Binned::update(double dt)
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
        if(!( cur.x < -100 || cur.x > ci::app::getWindowWidth()+100 || cur.y < -100 || cur.y > ci::app::getWindowHeight()+100 ))
        {
            
		// global force on other particles
		mParticleSystem.addRepulsionForce(cur, mParticleNeighborhood, mParticleRepulsion);
		// forces on this particle
        if( mBounceOffWalls )
        {
            const float padding = 50;
            cur.bounceOffWalls(0-padding, 0-padding, getWindowWidth()+padding, getWindowHeight()+padding, mWallDamping);
        }
        }
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
    
    switch (mApplyForcePattern) 
    {
        case PATTERN_FOUR_CORNERS:
            {
            Vec2i center(mApp->getWindowWidth()/2, mApp->getWindowHeight()/2);
            const float radius = mMaxRadius*0.5f;
            const float force = mMaxForce*0.5f;
            float d = radius;
            
            mParticleSystem.addRepulsionForce(center.x+d, center.y+d, radius, force*mForceScaleX, force*mForceScaleY);
            mParticleSystem.addRepulsionForce(center.x-d, center.y-d, radius, force*mForceScaleX, force*mForceScaleY);
            
            mParticleSystem.addRepulsionForce(center.x+d, center.y-d, radius, force*mForceScaleX, force*mForceScaleY);
            mParticleSystem.addRepulsionForce(center.x-d, center.y+d, radius, force*mForceScaleX, force*mForceScaleY);
            }
            break;
            
        case PATTERN_BPM_BOUNCE:
        {
            mBpmBounceTime -= mApp->getElapsedSecondsThisFrame();
            if( mBpmBounceTime <= 0.0f )
            {
                mBpmBounceTime = 60.0f / 126.0f; // 126 bpm
                Vec2i center;
                
                float radius = mMaxRadius*0.5f;
                const float force = mMaxForce;//*0.5f;
                //float d = radius*1.5f;
                /*
                switch( mBpmBouncePosition )
                {
                    case 0:
                        center.x = mApp->getWindowWidth()/2 - radius*2.0f;
                        center.y = mApp->getWindowHeight()/2 + d;
                        break;
                
                    case 1:
                        center.x = mApp->getWindowWidth()/2 + radius*2.0f;
                        center.y = mApp->getWindowHeight()/2 + d;
                        break;
                        
                    case 2:
                        center.x = mApp->getWindowWidth()/2 - radius*2.0f;
                        center.y = mApp->getWindowHeight()/2 - d;
                        break;
                        
                    case 3:
                        center.x = mApp->getWindowWidth()/2 + radius*2.0f;
                        center.y = mApp->getWindowHeight()/2 - d;
                        break;
                }
                 */
                center.x = mApp->getWindowWidth()/2;
                center.y = mApp->getWindowHeight()/2;
                
                //radius *= (mBpmBouncePosition+1)*(mBpmBouncePosition+1);
                
                mParticleSystem.addRepulsionForce(center.x, center.y, radius, force*mForceScaleX, force*mForceScaleY);
                
                if( ++mBpmBouncePosition > 3 )
                {
                    mBpmBouncePosition = 0;
                }
            }
            break;
        }
        case PATTERN_CROSSING:
        {
            mBpmBounceTime -= mApp->getElapsedSecondsThisFrame();
            if( mBpmBounceTime <= 0.0f )
            {
                mBpmBounceTime = 60.0f / 126.0f; // 126 bpm
            }
            else
            {
                break;
            }
            
            const float radius = mMaxRadius*0.5f;
            const float force = mMaxForce*0.5f;
            
            for( int i = 0; i < 4; ++i )
            {
                float d = radius*0.8f;
                
                switch( i )
                {
                    case 0:
                        mCrossForcePoint[i].x += d;
                        //mCrossForcePoint[i].y += d;
                        break;
                        
                    case 1:
                        //mCrossForcePoint[i].x += d;
                        mCrossForcePoint[i].y -= d;
                        break;
                        
                    case 2:
                        mCrossForcePoint[i].x -= d;
                        //mCrossForcePoint[i].y -= d;
                        break;
                        
                    case 3:
                        //mCrossForcePoint[i].x -= d;
                        mCrossForcePoint[i].y += d;
                        break;
                }
                
                mParticleSystem.addRepulsionForce(mCrossForcePoint[i].x, mCrossForcePoint[i].y, radius, force*mForceScaleX, force*mForceScaleY);
                
                if( mCrossForcePoint[i].x > getWindowWidth() || mCrossForcePoint[i].x < 0.0f || mCrossForcePoint[i].y > getWindowHeight() || mCrossForcePoint[i].y < 0.0f )
                {
                    mApplyForcePattern = PATTERN_NONE;
                    break;
                }
            }
            break;
        }
            
        case PATTERN_RING:
        {
            bool gotime = false;
            //float desired_fps = 25.0f;
            //float actual_fps = mApp->getAverageFps();
            //float scaled_elapsed_frame_time = mApp->getElapsedSecondsThisFrame() * (actual_fps/desired_fps);
            float scaled_elapsed_frame_time = 1.0f/25.0f;
            mBpmBounceTime -= scaled_elapsed_frame_time;//mApp->getElapsedSecondsThisFrame();
            if( mBpmBounceTime <= 0.0f )
            {
                gotime = (mBeatCount == -1);
                mBpmBounceTime = 60.0f / 126.0f; // 126 bpm
                if( ++mBeatCount == 4 )
                {
                    mBeatCount = 0;
                    gotime = true;
                }
            }
            
            if( gotime )
            {
                const float radius = mMaxRadius*0.5f;
                const float force = mMaxForce*0.5f;
                float theta = 0.0f;
                const float delta = (M_PI*2.0f)/180.f;
                static bool alternate = true;
                
                Vec2i center(mApp->getWindowWidth()/2, mApp->getWindowHeight()/2);
                
                //if( alternate )
                {
                
                for( theta = 0.0f; theta < (M_PI*2.0f); theta += delta )
                {
                    mParticleSystem.addRepulsionForce(center.x+sin(theta)*radius*2.0f, center.y+cos(theta)*radius*2.0f, radius, force*mForceScaleX, force*mForceScaleY);
                    
                }
                }
                //else
                {
                
                const float inner_force = force*1.25f;
                const float inner_radius = radius*1.f;
                mParticleSystem.addRepulsionForce(center.x, center.y, inner_radius, inner_force*mForceScaleX, inner_force*mForceScaleY);
                }
                alternate = !alternate;
            }
            break;
        }
        
        case PATTERN_NONE:
        default:
            break;
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
	mParticleSystem.draw( mPointColor, mParticleRadius );
    
    gl::enableDepthRead();
    gl::enableAlphaBlending();
    gl::popMatrices();
    
    ++mFrameCounter;
}

void Binned::updateAudioResponse()
{
    if( mAudioSensitivity == 0.0f )
    {
        return;
    }
    
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
            
        case 'v':
            mApplyForcePattern = (mApplyForcePattern == PATTERN_FOUR_CORNERS) ? PATTERN_NONE : PATTERN_FOUR_CORNERS;
            break;
            
        case 'b':
            mApplyForcePattern = (mApplyForcePattern == PATTERN_BPM_BOUNCE ) ? PATTERN_NONE : PATTERN_BPM_BOUNCE;
            mBpmBounceTime = 0.0f;
            mBeatCount = -1;
            break;
            
        case 'x':
            mApplyForcePattern = (mApplyForcePattern == PATTERN_CROSSING ) ? PATTERN_NONE : PATTERN_CROSSING;
            for( int i = 0; i < 4; ++i )
            {
                mCrossForcePoint[i] = Vec2i(mApp->getWindowWidth()/2, mApp->getWindowHeight()/2);
            }
            break;
            
        case 'n':
            mApplyForcePattern = (mApplyForcePattern == PATTERN_RING ) ? PATTERN_NONE : PATTERN_RING;
            mBpmBounceTime = 0.0f;
            mBeatCount = -1;
            break;
            
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
    //mMousePos = Vec2i(mApp->getWindowWidth()/2, mApp->getWindowHeight()/2);
}

void Binned::handleMouseUp( const MouseEvent& event )
{
	mIsMousePressed = false;
}

void Binned::handleMouseDrag( const MouseEvent& event )
{
	mMousePos = Vec2i(event.getPos());
    //mMousePos = Vec2i(mApp->getWindowWidth()/2, mApp->getWindowHeight()/2);
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
