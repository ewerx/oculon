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
#include "cinder/Easing.h"
#include "OculonApp.h"
#include "Constants.h"

using namespace ci;
using namespace ci::app;
using namespace bps;

Binned::Binned()
: Scene("Binned")
, mMode(MODE_BOTH)
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
    mKParticles = 6;
    
    mParticleRadius = 5.0f;
    
    mTimeStep = 0.005f;// 0.05
	mSlowMotion = false;
    mBounceOffWalls = true;
    mCircularWall = true;
	mParticleNeighborhood = 14;
    
    mCircularWallRadius = (mApp->getViewportWidth()/2)*1.2f;
    
	mParticleRepulsion = 1.5;
	mCenterAttraction = 0.94f;//0.03f;//0.05;
    
    mDamping = 0.01f;
    mWallDamping = 0.3f;
    
    mMinForce = 0.0f;
    mMinRadius = 0.0f;
    mMaxForce = 200.0f;
    mMaxRadius = 100.0f;
    mAudioSensitivity = 0.0f;
    
    mIsMousePressed = false;
    mApplyForcePattern = PATTERN_NONE;
    mPatternDuration = 0.0f;
    
    mPointColor.r = 1.0f;
    mPointColor.g = 0.0f;
    mPointColor.b = 0.0f;
    mPointColor.a = 0.5f;
    
    mForceColor.r = 0.85f;
    mForceColor.g = 0.0f;
    mForceColor.b = 0.0f;
    mForceColor.a = 0.12f;
    
    mParticleSystem.setForceColor( &mForceColor );
    
    mIsOrbiterModeEnabled = true;
    
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
	mParticleSystem.setup(mApp->getViewportWidth(), mApp->getViewportHeight(), binPower, this);
	
	float padding = 0;
	float maxVelocity = .5;
    
    bool ringformation = false;
    if( ringformation )
    {
        
        padding = mApp->getViewportWidth()*0.2f;
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
        const float centerX = mApp->getViewportWidth()/2.0f;
        const float centerY = mApp->getViewportHeight()/2.0f;
        const float thickness = mApp->getViewportWidth()*0.15f;
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
            float x = Rand::randFloat(padding, mApp->getViewportWidth() - padding);
            float y = Rand::randFloat(padding, mApp->getViewportHeight() - padding);
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
    params.addParam("Wall Radius", &mCircularWallRadius, "");
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
    
    snprintf(buf, 256, "mode: %s", mIsOrbiterModeEnabled ? "orbiter" : ( mCircularWall ? "circular" : "random" ));
    mApp->getInfoPanel().addLine(buf, Color(0.75f, 0.75f, 0.75f));
    
    snprintf(buf, 256, "pattern: %d", mApplyForcePattern);
    mApp->getInfoPanel().addLine(buf, Color(0.75f, 0.75f, 0.75f));
    
    if( mPatternDuration > 0.0f )
    {
        float time = (mPatternDuration*kCaptureFramerate - mFrameCounter) * dt;
        snprintf(buf, 256, "est. remaining: %.1fs", time);
        mApp->getInfoPanel().addLine(buf, Color(0.75f, 0.75f, 0.75f));
        snprintf(buf, 256, "actual elapsed: %.1fs", mFrameCounter/kCaptureFramerate);
        mApp->getInfoPanel().addLine(buf, Color(0.75f, 0.75f, 0.75f));
    }
}

void Binned::draw()
{
    gl::pushMatrices();
	gl::setMatricesWindow( mApp->getViewportWidth(), mApp->getViewportHeight() );
    
    gl::disableDepthRead();
    //gl::disableDepthWrite();
    glDisable(GL_LIGHTING);
	gl::enableAdditiveBlending();
	//glColor4f(1.0f, 1.0f, 1.0f, mLineOpacity);
	
	mParticleSystem.setupForces();
	// apply per-particle forces
    const float radius_sq = mCircularWallRadius*mCircularWallRadius;
    
	glBegin(GL_LINES);
	for(int i = 0; i < mParticleSystem.size(); i++) 
    {
		Particle& cur = mParticleSystem[i];
        if(!( cur.x < -100 || cur.x > mApp->getViewportWidth()+100 || cur.y < -100 || cur.y > mApp->getViewportHeight()+100 ))
        {
            // global force on other particles
            mParticleSystem.addRepulsionForce(cur, mParticleNeighborhood, mParticleRepulsion);
            // forces on this particle
            if( mBounceOffWalls )
            {
                if( mCircularWall )
                {
                    cur.bounceOffCircularWall(Vec2f( mApp->getViewportWidth()/2.0f, mApp->getViewportHeight()/2.0f ), mCircularWallRadius, radius_sq, mWallDamping);
                }
                else
                {
                    const float padding = 50;
                    cur.bounceOffWalls(0-padding, 0-padding, mApp->getViewportWidth()+padding, mApp->getViewportHeight()+padding, mWallDamping);
                }
            }
        }
		cur.addDampingForce(mDamping);
	}
	glEnd();
	// single global forces
	mParticleSystem.addAttractionForce(mApp->getViewportWidth()/2, mApp->getViewportHeight()/2, mApp->getViewportWidth(), mCenterAttraction);
	if(mIsMousePressed)
    {
        const float radius = mMaxRadius;
        const float force = mMaxForce;
        
        mParticleSystem.addRepulsionForce(mMousePos.x, mMousePos.y, radius, force*mForceScaleX, force*mForceScaleY);
    }
    
    switch (mApplyForcePattern) 
    {
        case PATTERN_FOUR_CORNERS:
            {
            Vec2i center(mApp->getViewportWidth()/2, mApp->getViewportHeight()/2);
            const float radius = mMaxRadius;
            const float force = mMaxForce;
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
                
                float radius = mMaxRadius;
                const float force = mMaxForce;//*0.5f;
                //float d = radius*1.5f;
                /*
                switch( mBpmBouncePosition )
                {
                    case 0:
                        center.x = mApp->getViewportWidth()/2 - radius*2.0f;
                        center.y = mApp->getViewportHeight()/2 + d;
                        break;
                
                    case 1:
                        center.x = mApp->getViewportWidth()/2 + radius*2.0f;
                        center.y = mApp->getViewportHeight()/2 + d;
                        break;
                        
                    case 2:
                        center.x = mApp->getViewportWidth()/2 - radius*2.0f;
                        center.y = mApp->getViewportHeight()/2 - d;
                        break;
                        
                    case 3:
                        center.x = mApp->getViewportWidth()/2 + radius*2.0f;
                        center.y = mApp->getViewportHeight()/2 - d;
                        break;
                }
                 */
                center.x = mApp->getViewportWidth()/2;
                center.y = mApp->getViewportHeight()/2;
                
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
                
                if( mCrossForcePoint[i].x > mApp->getViewportWidth() || mCrossForcePoint[i].x < 0.0f || mCrossForcePoint[i].y > mApp->getViewportHeight() || mCrossForcePoint[i].y < 0.0f )
                {
                    mApplyForcePattern = PATTERN_NONE;
                    break;
                }
            }
            break;
        }
            
        case PATTERN_RING:
        {
            /*
            const float elapsed_seconds = (mFrameCounter/kCaptureFramerate);
            float time = elapsed_seconds / mPatternDuration;
            if( time > 1.0f )
            {
                mApplyForcePattern = PATTERN_NONE;
                mApp->enableFrameCapture( false );
                break;
                //mApp->quit();
            }
             */
            
            bool gotime = true;
            //float desired_fps = kCaptureFramerate;
            //float actual_fps = mApp->getAverageFps();
            //float scaled_elapsed_frame_time = mApp->getElapsedSecondsThisFrame() * (actual_fps/desired_fps);
            float scaled_elapsed_frame_time = 1.0f/kCaptureFramerate;
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
                const float radius = Rand::randFloat(mMaxRadius*0.5f,mMaxRadius*1.5f);//mMaxRadius*0.5f;
                const float force = Rand::randFloat(mMaxForce*0.75f,mMaxForce*1.25f);
                float theta = 0.0f;
                const float delta = (M_PI*2.0f)/90.f;
                static bool alternate = true;
                
                Vec2i center(mApp->getViewportWidth()/2, mApp->getViewportHeight()/2);
                bool both = Rand::randFloat(1.0f) < 0.75f;
                if( alternate || both )
                {
                
                    for( theta = 0.0f; theta < (M_PI*2.0f); theta += delta )
                    {
                        mParticleSystem.addRepulsionForce(center.x+sin(theta)*radius*2.0f, center.y+cos(theta)*radius*2.0f, radius, force*mForceScaleX, force*mForceScaleY);
                        
                    }
                }                
                if( !alternate || both )
                {
                
                    const float inner_force = Rand::randFloat(0.5f*force,force*1.5f);
                    const float inner_radius = Rand::randFloat(radius*0.25f,radius);
                    mParticleSystem.addRepulsionForce(center.x, center.y, inner_radius, inner_force*mForceScaleX, inner_force*mForceScaleY);
                }
                alternate = !alternate;
                
                mApplyForcePattern = PATTERN_NONE;
            }
            break;
        }
            
        case PATTERN_DISSOLVE:
        {
            const float maxVal = 2.0f;
            const float elapsed_seconds = (mFrameCounter/kCaptureFramerate);
            float time = elapsed_seconds / mPatternDuration;
            if( time > 1.0f )
            {
                mApplyForcePattern = PATTERN_NONE;
                mApp->enableFrameCapture( false );
                mApp->quit();
            }
            else if( time > 0.5f )
            {
                mAudioSensitivity = math<float>::clamp( maxVal - maxVal * EaseOutInQuad()(time), 0.0f, maxVal );
            }
            else
            {
                mAudioSensitivity = math<float>::clamp( maxVal * EaseOutInQuad()(time), 0.0f, maxVal );
            }
            
            break;
        }
        
        case PATTERN_NONE:
        default:
            break;
    }
    
    if( mIsOrbiterModeEnabled )
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

void Binned::drawDebug()
{
    //gl::pushMatrices();
	//gl::setMatricesWindow( mApp->getWindowWidth(), mApp->getWindowHeight() );
    gl::disableDepthRead();
    //gl::disableDepthWrite();
    glDisable( GL_LIGHTING );
    glDisable( GL_TEXTURE_2D );
	gl::enableAdditiveBlending();
    //gl::enableAlphaBlending();
    
    Vec2f center( mApp->getWindowWidth()/2.0f, mApp->getWindowHeight()/2.0f );
    float scale = (float)(mApp->getWindowWidth()) / (float)(mApp->getViewportWidth()); 
    
    glColor4f(1.0f,0.0f,0.0f,0.5f);
    gl::drawStrokedCircle(center, mMaxRadius*scale);
    glColor4f(1.0f,0.2f,0.2f,0.25f);
    gl::drawStrokedCircle(center, mMaxRadius*0.75f*scale);
    gl::drawStrokedCircle(center, mMaxRadius*1.25f*scale);
    //gl::drawStrokedCircle(center, mApp->getWindowWidth()/3.0f);
    
    glColor4f(0.0f,0.2f,0.5f,0.5f);
    gl::drawStrokedCircle(center, mCircularWallRadius*scale);
    
    gl::enableDepthRead();
    //gl::enableDepthWrite();
    gl::enableAlphaBlending();
    //gl::popMatrices();
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
    
    int spacing = mApp->getViewportWidth() / bandCount;
    spacing *=2;
    int x1 = mApp->getViewportWidth() / 2;
    int x2 = x1;
    //int numBandsPerBody = 1;
    
    if( fftBuffer )
    {
        for( int i = 0; i < bandCount; ++i) 
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
                x1 = Rand::randFloat(0,mApp->getViewportWidth());
                x2 = Rand::randFloat(0,mApp->getViewportWidth());
            }
            const float magX = force * mForceScaleX;
            const float magY = force * mForceScaleY;
            const float centerY = mRandomPlacement ? Rand::randFloat(0,mApp->getViewportHeight()) :(mApp->getViewportHeight()/2.0f);
            
            if( mTopBottom )
            {
                mParticleSystem.addRepulsionForce(x1, 0, radius, magX, magY);
                mParticleSystem.addRepulsionForce(x2, 0, radius, magX, magY);
            
                mParticleSystem.addRepulsionForce(x1, mApp->getViewportHeight(), radius, magX, magY);
                mParticleSystem.addRepulsionForce(x2, mApp->getViewportHeight(), radius, magX, magY);
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
            //mApplyForcePattern = (mApplyForcePattern == PATTERN_BPM_BOUNCE ) ? PATTERN_NONE : PATTERN_BPM_BOUNCE;
            //mBpmBounceTime = 0.0f;
            //mBeatCount = -1;
            toggleActiveVisible();
            break;
            
        case 'o':
            mIsOrbiterModeEnabled = !mIsOrbiterModeEnabled;
            if( mIsOrbiterModeEnabled )
            {
                mAudioSensitivity = 0.0f;
                mCircularWall = true;
                mCenterAttraction = 0.9f;
                mRandomPlacement = false;
            }
            else
            {
                mAudioSensitivity = 1.0f;
                mCircularWall = false;
                mCenterAttraction = 0.05f;
                mRandomPlacement = true;
            }
            reset();
            break;
            
        case 'g':
            if( !mIsOrbiterModeEnabled )
            {
                if( mCircularWall )
                {
                    mAudioSensitivity = 0.8f;
                    mMaxRadius = 80.0f;
                    mCircularWall = false;
                    mRandomPlacement = true;
                    mCenterAttraction = 0.0f;
                }
                else
                {
                    mMaxRadius = 300.0f;
                    mCircularWall = true;
                    mAudioSensitivity = 0.0f;
                    mRandomPlacement = false;
                    mCenterAttraction = 0.9f;
                }
                reset();
            }
            break;
            
        case 'x':
            mApplyForcePattern = (mApplyForcePattern == PATTERN_CROSSING ) ? PATTERN_NONE : PATTERN_CROSSING;
            for( int i = 0; i < 4; ++i )
            {
                mCrossForcePoint[i] = Vec2i(mApp->getViewportWidth()/2, mApp->getViewportHeight()/2);
            }
            break;
            
        case 'N':
            mApp->enableFrameCapture( (mApplyForcePattern != PATTERN_RING) );
            // pass-thru
        case 'n':
            mApplyForcePattern = (mApplyForcePattern == PATTERN_RING ) ? PATTERN_NONE : PATTERN_RING;
            mBpmBounceTime = 0.0f;
            mBeatCount = -1;
            mFrameCounter = 0;
            mPatternDuration = 1.0f;
            break;
            
        
        case 'D':
            mApp->enableFrameCapture( (mApplyForcePattern != PATTERN_DISSOLVE) );
            // pass-thru
//        case 'd':
//            mApplyForcePattern = (mApplyForcePattern == PATTERN_DISSOLVE) ? PATTERN_NONE : PATTERN_DISSOLVE;
//            mFrameCounter = 0;
//            mPatternDuration = 60.0f;
//            break;
            
        case ' ':
            reset();
            break;
            
        case 'w':
            mPointColor.r = 1.0f;
            mPointColor.g = 1.0f;
            mPointColor.b = 1.0f;
            mPointColor.a = 0.5f;
            
            mForceColor.r = 0.85f;
            mForceColor.g = 0.85f;
            mForceColor.b = 0.85f;
            mForceColor.a = 0.12f;
            break;
            
        case 'd':
            mPointColor.r = 1.0f;
            mPointColor.g = 0.0f;
            mPointColor.b = 0.0f;
            mPointColor.a = 0.5f;
            
            mForceColor.r = 0.85f;
            mForceColor.g = 0.0f;
            mForceColor.b = 0.0f;
            mForceColor.a = 0.12f;
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
    //mMousePos = Vec2i(mApp->getViewportWidth()/2, mApp->getViewportHeight()/2);
}

void Binned::handleMouseUp( const MouseEvent& event )
{
	mIsMousePressed = false;
}

void Binned::handleMouseDrag( const MouseEvent& event )
{
	mMousePos = Vec2i(event.getPos());
    //mMousePos = Vec2i(mApp->getViewportWidth()/2, mApp->getViewportHeight()/2);
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
