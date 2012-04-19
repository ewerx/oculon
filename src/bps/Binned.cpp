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
#include "Interface.h"

using namespace ci;
using namespace ci::app;
using namespace bps;

Binned::Binned()
: Scene("Binned")
, mRepulsionMode(MODE_BOTH)
, mForceScaleX(1.0f)
, mForceScaleY(1.0f)
{
}

Binned::~Binned()
{
}

void Binned::setup()
{
    mKParticles = 16;
    mParticleNeighborhood = 14;
    
    mPointSize = 6.0f;
    mLineWidth = 2.0f;
    
    mTimeStep = 0.005f;// 0.05
	mSlowMotion = false;
    mBounceOffWalls = false;
    mCircularWall = false;
    mWallPadding = 0.0f;
    
    mCircularWallRadius = (mApp->getViewportWidth()/2)*1.2f;
    
    mInitialFormation = FORMATION_NONE;
    
	mParticleRepulsion = 0.5f;//1.5;
	mCenterAttraction = 0.08f;//0.05;
    
    mDamping = 0.0f;//0.01f;
    mWallDamping = 0.8f;//0.3f;
    
    mMinForce = 0.0f;
    mMinRadius = 0.0f;
    mMaxForce = 200.0f;
    mMaxRadius = 100.0f;
    mAudioSensitivity = 0.0f;
    
    mIsMousePressed = false;
    mApplyForcePattern = PATTERN_NONE;
    mPatternDuration = 0.0f;
    mAudioPattern = AUDIO_PATTERN_SEGMENTS;
    
    mPointColor.r = 1.0f;
    mPointColor.g = 1.0f;
    mPointColor.b = 1.0f;
    mPointColor.a = 0.5f;
    
    mForceColor.r = 0.85f;
    mForceColor.g = 0.85f;
    mForceColor.b = 0.85f;
    mForceColor.a = 0.12f;
    
    mParticleSystem.setForceColor( &mForceColor );
    
    mIsOrbiterModeEnabled = false;
    
    // deepfield
    mNumSegments = 4;
    mSegmentWidth = mApp->getViewportWidth()/4.0f;
    
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
    
    switch( mInitialFormation )
    {
        case FORMATION_RING:
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
            break;
            
        case FORMATION_DISC:
        {
            const float centerX = mApp->getViewportWidth()/2.0f;
            const float centerY = mApp->getViewportHeight()/2.0f;
            const float radius = mApp->getViewportWidth() * 0.5f;
            for(int i = 0; i < mKParticles * 1024; i++) 
            {
                float r = Rand::randFloat(0.0f,M_PI*4.0f);
                float x = centerX + sin(r)*Rand::randFloat(radius);
                float y = centerY + cos(r)*Rand::randFloat(radius);
                float xv = Rand::randFloat(-maxVelocity*mForceScaleX, maxVelocity*mForceScaleX);
                float yv = Rand::randFloat(-maxVelocity*mForceScaleY, maxVelocity*mForceScaleY);
                mParticleSystem.add(Particle(x, y, xv, yv));
            }
        }
            break;
        case FORMATION_NONE:
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
            break;
            
        case FORMATION_WAVE:
        {
            const float thickness = mApp->getViewportWidth() * 0.05f;
            for(int i = 0; i < mKParticles * 1024; i++) 
            {
                float x = Rand::randFloat(0, thickness);
                float y = Rand::randFloat(0, mApp->getViewportHeight());
                float xv = Rand::randFloat(800, 1000);
                float yv = Rand::randFloat(-maxVelocity*10.0f, maxVelocity*10.0f);
                Particle particle(x, y, xv, yv);
                mParticleSystem.add(particle);
            }
        }
            break;
            
        case FORMATION_MULTIWAVE:
        {
            const float thickness = mApp->getViewportWidth() * 0.05f;
            const int numWaves = 4;
            const int ppw = (mKParticles * 1024) / numWaves;
            const float waveDist = thickness * 36.0f;
            float pos = 0;
            for(int w = 0; w < numWaves; ++w)
            {
                for(int i = w*ppw; i < (w+1)*ppw; ++i) 
                {
                    float x = Rand::randFloat(pos, pos+thickness);
                    float y = Rand::randFloat(0, mApp->getViewportHeight());
                    float xv = Rand::randFloat(800, 1000);
                    float yv = Rand::randFloat(-maxVelocity*10.0f, maxVelocity*10.0f);
                    Particle particle(x, y, xv, yv);
                    mParticleSystem.add(particle);
                }
                pos -= waveDist;
            }
        }
            break;
            
        default:
            break;
    }
    
    while( !mQueuedForces.empty() )
    {
        mQueuedForces.pop();
    }
}

void Binned::resize()
{
    reset();
}

void Binned::setupDebugInterface()
{
    Scene::setupDebugInterface(); // add all interface params
    
    mDebugParams.addSeparator();
    mDebugParams.addParam("Time Step", &mTimeStep, "step=0.001 min=0.0001 max=1.0");
    mDebugParams.addParam("Wall Bounce", &mBounceOffWalls, "");
    mDebugParams.addParam("Particle Repulsion", &mParticleRepulsion, "step=0.01");
    mDebugParams.addParam("Damping Force", &mDamping, "step=0.01");
    mDebugParams.addParam("Wall Damping", &mWallDamping, "step=0.01");
    mDebugParams.addParam("Wall Radius", &mCircularWallRadius, "");
    mDebugParams.addParam("Center Attraction", &mCenterAttraction, "step=0.01");
    mDebugParams.addParam("Force Scale X", &mForceScaleX, "step=0.1");
    mDebugParams.addParam("Force Scale Y", &mForceScaleY, "step=0.1");
    mDebugParams.addParam("Min Force", &mMinForce, "");
    mDebugParams.addParam("Max Force", &mMaxForce, "");
    mDebugParams.addParam("Min Radius", &mMinRadius, "");
    mDebugParams.addParam("Max Radius", &mMaxRadius, "");
    mDebugParams.addParam("Audio Sensitivity", &mAudioSensitivity, "step=0.01 min=0.0");
    mDebugParams.addParam("K Particles", &mKParticles, "min=1 max=100");
    mDebugParams.addParam("Point Color", &mPointColor, "");
    mDebugParams.addParam("Force Color", &mForceColor, "");
    
    
}

void Binned::setupInterface()
{
    mInterface->addParam(CreateIntParam("Initial Formation", &mInitialFormation)
                         .minValue(0)
                         .maxValue(FORMATION_COUNT-1));
    mInterface->addParam(CreateFloatParam("Wall Padding", &mWallPadding)
                         .minValue(-10.0f)
                         .maxValue(50.0f));
    
    
    mDebugParams.addParam("Mode", &mRepulsionMode, "");
    mDebugParams.addParam("Time Step", &mTimeStep, "step=0.001 min=0.0001 max=1.0");
    
    mDebugParams.addParam("Center Attraction", &mCenterAttraction, "step=0.01");
    
    mDebugParams.addParam("Force Scale X", &mForceScaleX, "step=0.1");
    mDebugParams.addParam("Force Scale Y", &mForceScaleY, "step=0.1");
    mDebugParams.addParam("Min Force", &mMinForce, "");
    mDebugParams.addParam("Max Force", &mMaxForce, "");
    mDebugParams.addParam("Min Radius", &mMinRadius, "");
    mDebugParams.addParam("Max Radius", &mMaxRadius, "");

    
    mDebugParams.addParam("Wall Bounce", &mBounceOffWalls, "");
    mDebugParams.addParam("Point Size", &mPointSize, "min=1 max=20");
    mDebugParams.addParam("Line Width", &mLineWidth, "min=1 max=10");
    

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
    glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, mPointSize );
    glPointParameterfARB( GL_POINT_SIZE_MIN_ARB, mPointSize );
    glDisable(GL_POINT_SPRITE_ARB);
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
    glEnable(GL_POINT_SPRITE);
    glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glPointSize(mPointSize);
    glLineWidth(mLineWidth);
	
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
                    cur.bounceOffWalls(0-mWallPadding, 0-mWallPadding, mApp->getViewportWidth()+mWallPadding, mApp->getViewportHeight()+mWallPadding, mWallDamping);
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
                
                //mApplyForcePattern = PATTERN_NONE;
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
            if( mIsOrbiterModeEnabled )
            {
                applyQueuedForces();
            }
            else
            {
                updateAudioResponse();
            }
        default:
            break;
    }
    
	mParticleSystem.update();
	//glColor4f(1.0f, 1.0f, 1.0f, mPointOpacity);
	mParticleSystem.draw( mPointColor, mPointSize );
    
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
    
    Scene::drawDebug();
}

void Binned::updateAudioResponse()
{
    if( mAudioSensitivity == 0.0f )
    {
        return;
    }
    AudioInput& audioInput = mApp->getAudioInput();
    float * freqData = audioInput.getFft()->getAmplitude();
    //float * timeData = audioInput.getFft()->getData();
    int32_t dataSize = audioInput.getFft()->getBinSize();
    
    const float width = mApp->getViewportWidth();
    const float height = mApp->getViewportHeight();
    const float centerX = width/2.0f;
    const float centerY = height/2.0f;
    const int horizSpacing = (width / dataSize) * 2;
    
    // audio data
    for (int32_t i = 0; i < dataSize; ++i) 
    {
        // logarithmic plotting for frequency domain
        double logSize = log((double)dataSize);
        const float normalizedFreq = (float)(log((double)i) / logSize);
        const float freq = normalizedFreq * (double)dataSize;
        const float amp = mAudioSensitivity * math<float>::clamp(freqData[i] * (freq / dataSize) * log((double)(dataSize - i)), 0.0f, 2.0f);
        
        float radius = mMinRadius;
        float force = mMinForce;
        
        switch( mRepulsionMode )
        {
            case MODE_RADIUS:
                radius += mMaxRadius * amp;
                force = mMaxForce / 2.0f;
                break;
            case MODE_FORCE:
                radius = mMinRadius * 5.0f;
                force += mMaxForce * amp;
                break;
            case MODE_BOTH:
                radius += mMaxRadius * amp;
                force += mMaxForce * amp;
                break;
            default:
                assert(false && "invalid mode");
                break;
        }
        
        const float magX = force * mForceScaleX;
        const float magY = force * mForceScaleY;
        
        switch( mAudioPattern )
        {
            case AUDIO_PATTERN_LINE:
            {
                const float x1 = centerX + i*horizSpacing;
                const float x2 = centerX - i*horizSpacing;
                mParticleSystem.addRepulsionForce(x1, centerY, radius, magX, magY);
                mParticleSystem.addRepulsionForce(x2, centerY, radius, magX, magY);
            }
                break;
            case AUDIO_PATTERN_TOPBOTTOM:
            {
                const float x1 = centerX + i*horizSpacing;
                const float x2 = centerX - i*horizSpacing;
                
                mParticleSystem.addRepulsionForce(x1, 0, radius, magX, magY);
                mParticleSystem.addRepulsionForce(x2, 0, radius, magX, magY);
                
                mParticleSystem.addRepulsionForce(x1, height, radius, magX, magY);
                mParticleSystem.addRepulsionForce(x2, height, radius, magX, magY);
            }
                break;
                
            case AUDIO_PATTERN_RANDOM:
            {
                const float x1 = Rand::randFloat(0,width);
                const float x2 = Rand::randFloat(0,width);
                const float y1 = Rand::randFloat(0,height);
                const float y2 = Rand::randFloat(0,height);
                
                mParticleSystem.addRepulsionForce(x1, y1, radius, magX, magY);
                mParticleSystem.addRepulsionForce(x2, y2, radius, magX, magY);
            }
                break;
                
            case AUDIO_PATTERN_SEGMENTS:
            {
                const int numFreqSections = (mNumSegments+1) * 2; // two sections per divider
                const int freqSection = Rand::randInt(0,numFreqSections);
                
                const float x = freqSection * mSegmentWidth;
                const float y = Rand::randFloat(0, height);
                mParticleSystem.addRepulsionForce( x, y, radius, magX, magY );
                
                bool debug = true;
                if( debug )
                {
                    const float maxRadius = 50.0f;
                    
                    glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
                    gl::drawSolidCircle(Vec2f(x, y), maxRadius * amp);
                }

            }
                break;
                
            case AUDIO_PATTERN_COUNT:
                break;
        }
            
//            bool debug = false;
//            if( debug )
//            {
//                const float maxRadius = 50.0f;
//                
//                glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
//                gl::drawSolidCircle(Vec2f(x1, centerY), maxRadius * avgFft);
//                gl::drawSolidCircle(Vec2f(x2, centerY), maxRadius * avgFft);
//            }
    }
}

bool Binned::handleKeyDown( const KeyEvent& event )
{
    bool handled = true;
    
	switch( event.getChar() )
    {
        case 's':
            mTimeStep *= 0.1f;
            break;
        case 'S':
            mTimeStep *= 10.0f;
            break;
            
        case 'm':
        {
            int mode = mRepulsionMode;
            mode++;
            if( mode >= MODE_COUNT )
                mode = 0;
            mRepulsionMode = static_cast<eRepulsionMode>(mode);
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
            }
            else
            {
                mAudioSensitivity = 1.0f;
                mCircularWall = false;
                mCenterAttraction = 0.05f;
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
                    mCenterAttraction = 0.0f;
                }
                else
                {
                    mMaxRadius = 300.0f;
                    mCircularWall = true;
                    mAudioSensitivity = 0.0f;
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

