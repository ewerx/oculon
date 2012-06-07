/*
 *  Tectonic.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Tectonic.h"
#include "Quake.h"
#include "QuakeData.h"
#include "TextEntity.h"

#include "OculonApp.h"
#include "AudioInput.h"
#include "Interface.h"
#include "Resources.h"

#include "cinder/Rand.h"


using namespace ci;
using namespace ci::app;

// ----------------------------------------------------------------
//
Tectonic::Tectonic()
: Scene("tectonic")
, mDisplayListPoints(0)
{
    mData = new USGSQuakeData();
    
    for( int i=0; i < TB_COUNT; ++i )
    {
        mTextBox[i] = new TextEntity(this);
        assert(mTextBox[i] != NULL && "out of memory, how ridiculous");
    }
}

// ----------------------------------------------------------------
//
Tectonic::~Tectonic()
{
    clearQuakes();
    
    for( int i=0; i < TB_COUNT; ++i )
    {
        delete mTextBox[i];
        mTextBox[i] = NULL;
    }
    
    delete mData;
}

// ----------------------------------------------------------------
//
void Tectonic::setup()
{
    // params
    mTriggerMode = TRIGGER_BPM;
    mBpm = 120.0f;
    mLongitudeOffsetDegrees = 205; // pacific ocean centered
    mShowMap = false;
    mShowLabels = true;
    mShowAllPoints = false;
    
    // assets
    mEarthDiffuse = gl::Texture( loadImage( loadResource( RES_EARTHDIFFUSE ) ) );
    mEarthDiffuse.setWrap( GL_REPEAT, GL_REPEAT );
    
    //mData->parseData("http://earthquake.usgs.gov/earthquakes/catalogs/7day-M2.5.xml");
    //mData->parseData("http://earthquake.usgs.gov/earthquakes/feed/atom/2.5/month");
    mData->parseData("/Volumes/cruxpod/oculondata/month.xhtml");
    
    initQuakes();
    
    reset();
}

// ----------------------------------------------------------------
//
void Tectonic::initQuakes()
{
    clearQuakes();
    
    for (QuakeData::EventMap::const_iterator it = mData->eventsBegin();
         it != mData->eventsEnd();
         ++it)
    {
        const QuakeEvent* eventData = &((*it).second);
        mQuakes.push_back( new Quake(this, eventData, mLongitudeOffsetDegrees) );
        //console() << "Quake" << mQuakes.size() << ": " << eventData->toString() << std::endl;
    }
    
    // points display list
    mDisplayListPoints = glGenLists(1);
    glNewList(mDisplayListPoints, GL_COMPILE);
    glBegin(GL_POINTS);
    for(QuakeList::iterator it = mQuakes.begin(); 
        it != mQuakes.end();
        ++it)
    {
        const Vec3f& pos = (*it)->getPosition();
        glVertex2f(pos.x, pos.y);
    }
    glEnd();
    glEndList();
}

// ----------------------------------------------------------------
//
void Tectonic::clearQuakes()
{
    for(QuakeList::iterator it = mQuakes.begin(); 
        it != mQuakes.end();
        ++it)
    {
        delete (*it);
    }
    mQuakes.clear();
    
    if( mDisplayListPoints != 0 )
    {
        glDeleteLists(mDisplayListPoints, 1);
    }
}

// ----------------------------------------------------------------
//
void Tectonic::setupInterface()
{
    mInterface->addEnum(CreateEnumParam( "Trigger Mode", (int*)(&mTriggerMode) )
                        .maxValue(TRIGGER_COUNT)
                        .isVertical()
                        .oscReceiver(mName,"triggermode"));
    mInterface->addParam(CreateFloatParam("BPM", &mBpm)
                         .minValue(60.0f)
                         .maxValue(150.0f)
                         .oscReceiver(mName,"bpm")
                         .sendFeedback());    
    mInterface->addButton(CreateTriggerParam("Trigger Quake", NULL)
                          .oscReceiver(mName,"quaketrigger"))->registerCallback( this, &Tectonic::triggerNextQuake );
}

// ----------------------------------------------------------------
//
void Tectonic::setupDebugInterface()
{
    mDebugParams.addParam("Show Map", &mShowMap );
    mDebugParams.addParam("Show Labels", &mShowLabels );
    mDebugParams.addParam("Show All Points", &mShowAllPoints );
    mDebugParams.addParam("Longitude Offset", &mLongitudeOffsetDegrees );
}

// ----------------------------------------------------------------
//
void Tectonic::reset()
{
    mCurrentIndex = 0;
    mBpmTriggerTime = 60.0f / mBpm;
    mActiveQuakes.clear();
    mIsCapturing = false;
    
}

// ----------------------------------------------------------------
//
void Tectonic::resize()
{
    for( int i = 0; i < TB_COUNT; ++i )
    {
        mTextBox[i]->resize();
    }
}

// ----------------------------------------------------------------
//
void Tectonic::update(double dt)
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
        
        case TRIGGER_MANUAL:
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

// ----------------------------------------------------------------
//
void Tectonic::triggerAll()
{
    if( mActiveQuakes.empty() )
    {
        mActiveQuakes = mQuakes;
        
        for(QuakeList::iterator it = mActiveQuakes.begin(); 
            it != mActiveQuakes.end();
            ++it)
        {
            (*it)->trigger(0.0f);
        }
    }
}

// ----------------------------------------------------------------
//
void Tectonic::triggerByBpm(double dt)
{
    mBpmTriggerTime -= dt;
    if( mBpmTriggerTime <= 0.0f )
    {
        //mBpmTriggerTime = 60.0f / mBpm;
        
        triggerNextQuake();
    }
}

void Tectonic::triggerByTime(double dt)
{
    
}

bool Tectonic::triggerNextQuake()
{
    mBpmTriggerTime = 60.0f / mBpm;
    
    if( mCurrentIndex < mQuakes.size() )
    {
        if( !mIsCapturing || (60.0f-mApp->getFrameCaptureCount()/kCaptureFramerate > 5.0f) )
        {
            //console() << "active quakes: " << mActiveQuakes.size();
            //mActiveQuakes.clear();
            mActiveQuakes.erase(std::remove_if(mActiveQuakes.begin(), mActiveQuakes.end(), IsTriggeredQuakeFinished()), mActiveQuakes.end());
            //console() << " --> " << mActiveQuakes.size() << std::endl;
            assert(mQuakes[mCurrentIndex] != NULL);
            
            const float durationMagnitudeMultiplier = 0.25f;
            float duration = 60.0f / mBpm + durationMagnitudeMultiplier*mQuakes[mCurrentIndex]->getEventData()->getMag();
            if( mIsCapturing ) duration *= kCaptureFramerate / mApp->getAverageFps();
            mQuakes[mCurrentIndex]->trigger(duration);
            mActiveQuakes.push_back( mQuakes[mCurrentIndex] );
            console() << mCurrentIndex << ": " << mQuakes[mCurrentIndex]->getEventData()->toString() << std::endl;
        }
    }
    
    ++mCurrentIndex;
    if( mCurrentIndex >= mQuakes.size() )
    {
        if( mIsCapturing )
        {
            mApp->enableFrameCapture( false );
            mIsCapturing = false;
            return false;
        }
        mCurrentIndex = 0;
    }
    
    return false;
}

// ----------------------------------------------------------------
//
void Tectonic::draw()
{
    gl::pushMatrices();
    gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
    
    gl::enableAlphaBlending();
    glEnable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    
    drawEarthMap();
    drawPoints();
    drawQuakes();
    drawHud();
    
    gl::popMatrices();
}

// ----------------------------------------------------------------
//
void Tectonic::drawEarthMap()
{
    if( mShowMap )
    {
        gl::color( 1.0f, 1.0f, 1.0f, 1.0f );
        //gl::draw( mEarthDiffuse, Rectf( 0, 0, mApp->getViewportWidth(), mApp->getViewportHeight() ) );
        const float textureWidth = mEarthDiffuse.getCleanWidth();
        const float screenWidth = mApp->getViewportWidth();
        const float screenHeight = mApp->getViewportHeight();
        const float textureOffset = ((float)(mLongitudeOffsetDegrees)/360.0f) * textureWidth;
        
        gl::draw( mEarthDiffuse, Area( -textureOffset, 0, textureWidth-textureOffset, mEarthDiffuse.getCleanHeight() ), Rectf( 0, 0, screenWidth, screenHeight ) );
    }
}

// ----------------------------------------------------------------
//
void Tectonic::drawQuakes()
{
    gl::disableDepthRead();
    for(QuakeList::iterator it = mActiveQuakes.begin(); 
        it != mActiveQuakes.end();
        ++it)
    {
        (*it)->draw();
    }
    //gl::enableDepthRead();
}

// ----------------------------------------------------------------
//
void Tectonic::drawPoints()
{
    if( mShowAllPoints )
    {
        glCallList(mDisplayListPoints);
    }
}

// ----------------------------------------------------------------
//
void Tectonic::drawHud()
{
    gl::pushMatrices();
    
    const float width = mApp->getViewportWidth();
    const float height = mApp->getViewportHeight();
    
    //CameraOrtho textCam(0.0f, width, height, 0.0f, 0.0f, 10.f);
    //gl::setMatrices(textCam);
    
    if( mShowLabels )
    {
        for(QuakeList::iterator it = mActiveQuakes.begin(); 
            it != mActiveQuakes.end();
            ++it)
        {
            (*it)->drawLabel();
        }
    }
    
    gl::popMatrices();
}

// ----------------------------------------------------------------
//
void Tectonic::drawDebug()
{
    //gl::pushMatrices();
    //gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );

    //gl::popMatrices();
}

// ----------------------------------------------------------------
//
bool Tectonic::handleKeyDown(const KeyEvent& keyEvent)
{
    bool handled = true;
    
    switch (keyEvent.getCode()) 
    {
        case KeyEvent::KEY_SPACE:
            reset();
            handled = false;
            break;
            
        case KeyEvent::KEY_n:
            if( keyEvent.isShiftDown() )
            {
                mApp->enableFrameCapture( false );
                mIsCapturing = true;
                mApp->setCaptureDuration( 60.0f );
                mApp->enableFrameCapture( mIsCapturing );
                mCurrentIndex = 0;
            }
            break;
            
        default:
            handled = false;
            break;
    }
    
    return handled;
}