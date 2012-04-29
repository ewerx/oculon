/*
 *  Quaker.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Quaker.h"
#include "Quake.h"
#include "QuakeData.h"

#include "OculonApp.h"
#include "AudioInput.h"
#include "Interface.h"
#include "Resources.h"

#include "cinder/Rand.h"


using namespace ci;
using namespace ci::app;

Quaker::Quaker()
: Scene("Quaker")
{
    mData = new USGSQuakeData();
}

Quaker::~Quaker()
{
    clearQuakes();
    delete mData;
}

// ----------------------------------------------------------------
//
void Quaker::setup()
{
    mEarthDiffuse = gl::Texture( loadImage( loadResource( RES_EARTHDIFFUSE ) ) );
    
    initQuakes();
    
    // params
    mTriggerMode = TRIGGER_BPM;
    mBpm = 125.0f;
    
    reset();
}

// ----------------------------------------------------------------
//
void Quaker::initQuakes()
{
    clearQuakes();
    
    mData->parseData("http://earthquake.usgs.gov/earthquakes/catalogs/7day-M2.5.xml");
    //mData->parseData("http://earthquake.usgs.gov/earthquakes/feed/atom/2.5/month");
    
    for (QuakeData::EventMap::const_iterator it = mData->eventsBegin();
         it != mData->eventsEnd();
         ++it)
    {
        const QuakeEvent* eventData = &((*it).second);
        mQuakes.push_back( new Quake(this, eventData) );
        console() << "Quake" << mQuakes.size() << ": " << eventData->toString() << std::endl;
    }
}

void Quaker::clearQuakes()
{
    for(QuakeList::iterator it = mQuakes.begin(); 
        it != mQuakes.end();
        ++it)
    {
        delete (*it);
    }
    mQuakes.clear();
}

// ----------------------------------------------------------------
//
void Quaker::setupInterface()
{
    //mInterface->addParam(CreateBoolParam( "Motion Blur", &mUseMotionBlur )
    //                     .defaultValue(mUseMotionBlur));
}

// ----------------------------------------------------------------
//
void Quaker::reset()
{
    mCurrentIndex = 0;
    mBpmTriggerTime = 60.0f / mBpm;
    mActiveQuakes.clear();
}

// ----------------------------------------------------------------
//
void Quaker::update(double dt)
{
    switch (mTriggerMode)
    {
        case TRIGGER_ALL:
            triggerAll();
            break;
            
        case TRIGGER_BPM:
            triggerByBpm(dt);
            break;
            
        case TRIGGER_REALTIME:
            triggerByTime(dt);
            break;
        
        default:
            assert(false && "invalid trigger mode");
            break;
    }
    
    for(QuakeList::iterator it = mActiveQuakes.begin(); 
        it != mActiveQuakes.end();
        ++it)
    {
        (*it)->update(dt);
    }
    
    // last
    Scene::update(dt);
}

void Quaker::triggerAll()
{
    if( mActiveQuakes.empty() )
    {
        mActiveQuakes = mQuakes;
    }
}

void Quaker::triggerByBpm(double dt)
{
    mBpmTriggerTime -= mApp->getElapsedSecondsThisFrame();
    if( mBpmTriggerTime <= 0.0f )
    {
        mBpmTriggerTime = 60.0f / mBpm;
        
        if( mCurrentIndex < mQuakes.size() )
        {
            //console() << "active quakes: " << mActiveQuakes.size();
            //mActiveQuakes.clear();
            mActiveQuakes.erase(std::remove_if(mActiveQuakes.begin(), mActiveQuakes.end(), IsTriggeredQuakeFinished()), mActiveQuakes.end());
            //console() << " --> " << mActiveQuakes.size() << std::endl;
            assert(mQuakes[mCurrentIndex] != null);
            
            const float durationMagnitudeMultiplier = 0.25f;
            const float duration = 60.0f / mBpm + durationMagnitudeMultiplier*mQuakes[mCurrentIndex]->getEventData()->getMag();
            mQuakes[mCurrentIndex]->trigger(duration);
            mActiveQuakes.push_back( mQuakes[mCurrentIndex] );
            console() << mCurrentIndex << ": " << mQuakes[mCurrentIndex]->getEventData()->toString() << std::endl;
        }
        
        ++mCurrentIndex;
        if( mCurrentIndex >= mQuakes.size() )
        {
            mCurrentIndex = 0;
        }
    }
}

void Quaker::triggerByTime(double dt)
{
    
}

// ----------------------------------------------------------------
//
void Quaker::draw()
{
    gl::pushMatrices();
    gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
    
    gl::enableAlphaBlending();
    glEnable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    
    drawEarthMap();
    drawQuakes();
    
    gl::popMatrices();
}

// ----------------------------------------------------------------
//
void Quaker::drawEarthMap()
{
    gl::color( 1.0f, 1.0f, 1.0f, 1.0f );
    gl::draw( mEarthDiffuse, Rectf( 0, 0, mApp->getViewportWidth(), mApp->getViewportHeight() ) );
}

void Quaker::drawQuakes()
{
    gl::disableDepthRead();
    for(QuakeList::iterator it = mActiveQuakes.begin(); 
        it != mActiveQuakes.end();
        ++it)
    {
        (*it)->draw();
    }
    
    gl::pushMatrices();
    
    const float width = mApp->getViewportWidth();
    const float height = mApp->getViewportHeight();
    
    CameraOrtho textCam(0.0f, width, height, 0.0f, 0.0f, 10.f);
    gl::setMatrices(textCam);
    
    for(QuakeList::iterator it = mActiveQuakes.begin(); 
        it != mActiveQuakes.end();
        ++it)
    {
        (*it)->drawLabel();
    }
    
    gl::popMatrices();
}

// ----------------------------------------------------------------
//
void Quaker::drawDebug()
{
    gl::pushMatrices();
    gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );

    gl::popMatrices();
}

// ----------------------------------------------------------------
//
bool Quaker::handleKeyDown(const KeyEvent& keyEvent)
{
    bool handled = true;
    
    switch (keyEvent.getCode()) 
    {
        case KeyEvent::KEY_SPACE:
            reset();
            handled = false;
            break;
            
        default:
            handled = false;
            break;
    }
}