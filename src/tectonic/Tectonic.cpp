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
#include "Binned.h"

#include "cinder/Rand.h"


using namespace ci;
using namespace ci::app;

// ----------------------------------------------------------------
//
Tectonic::Tectonic()
: Scene("tectonic")
, mDisplayListPoints(0)
{
    mData[DATASOURCE_QUAKES] = new USGSQuakeData();
    mData[DATASOURCE_NUKES] = new NuclearEventData();
    
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
    
    for( int i=0; i < DATASOURCE_COUNT; ++i )
    {
        delete mData[i];
    }
}

// ----------------------------------------------------------------
//
void Tectonic::setup()
{
    Scene::setup();
    
    // params
    mTriggerMode = TRIGGER_BPM;
    mBpm = 120.0f;
    mBpmTapTimer = 0.0f;
    mLongitudeOffsetDegrees = 205; // pacific ocean centered
    mShowMap = false;
    mShowLabels = true;
    mShowAllPoints = false;
    mSendToBinned = true;
    mDataSource = DATASOURCE_QUAKES;
    mMarkerColor = Color(1.0f,0.0f,0.0f);
    mEarthAlpha = 0.0f;
    mColorNukesByType = true;
    mGenerateAudio = false;
    mTimeScale = 1.0f;
    
    // assets
    mEarthDiffuse = gl::Texture( loadImage( loadResource( "earthGray.png" ) ) );
    mEarthDiffuse.setWrap( GL_REPEAT, GL_REPEAT );
    
    //mData->parseData("http://earthquake.usgs.gov/earthquakes/catalogs/7day-M2.5.xml");
    //mData->parseData("http://earthquake.usgs.gov/earthquakes/feed/atom/2.5/month");
    mData[DATASOURCE_QUAKES]->parseData(App::getResourcePath("usgs_earthquakes_30days.xhtml").generic_string());
    mData[DATASOURCE_NUKES]->parseData(App::getResourcePath("nukes.csv").generic_string());
    
    reset();
}

// ----------------------------------------------------------------
//
void Tectonic::initEvents(const eDataSource src)
{
    clearQuakes();
    
    for (QuakeData::EventMap::const_iterator it = mData[src]->eventsBegin();
         it != mData[src]->eventsEnd();
         ++it)
    {
        //const QuakeEvent* eventData = &((*it).second);
        const QuakeEvent* eventData = &(*it);
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
    mInterface->addEnum(CreateEnumParam( "Data Source", (int*)(&mTriggerMode) )
                        .maxValue(DATASOURCE_COUNT)
                        .isVertical()
                        .oscReceiver(mName,"datasource"))->registerCallback( this, &Tectonic::changeDataSource );
    mInterface->addEnum(CreateEnumParam( "Trigger Mode", (int*)(&mTriggerMode) )
                        .maxValue(TRIGGER_COUNT)
                        .isVertical()
                        .oscReceiver(mName,"triggermode"));
    mInterface->addParam(CreateFloatParam("BPM", &mBpm)
                         .minValue(20.0f)
                         .maxValue(600.0f)
                         .oscReceiver(mName,"bpm")
                         .sendFeedback());    
    mInterface->addButton(CreateTriggerParam("Trigger Quake", NULL)
                          .oscReceiver(mName,"quaketrigger"))->registerCallback( this, &Tectonic::triggerNextQuake );
    mInterface->addButton(CreateTriggerParam("BPM Tap", NULL)
                          .oscReceiver(mName,"bpmtap"))->registerCallback( this, &Tectonic::bpmTap );
    mInterface->addParam(CreateFloatParam("Earth Alpha", &mEarthAlpha)
                         .oscReceiver(mName,"earthalpha"));
    mInterface->addParam(CreateFloatParam( "Time Scale", &mTimeScale )
                         .minValue(1.0f)
                         .maxValue(30.0f)
                         .oscReceiver(getName(), "timescale"));
}

// ----------------------------------------------------------------
//
//void Tectonic::setupDebugInterface()
//{
//    mDebugParams.addParam("Show Map", &mShowMap );
//    mDebugParams.addParam("Show Labels", &mShowLabels );
//    mDebugParams.addParam("Show All Points", &mShowAllPoints );
//    mDebugParams.addParam("Longitude Offset", &mLongitudeOffsetDegrees );
//}

// ----------------------------------------------------------------
//
void Tectonic::reset()
{
    initEvents(static_cast<eDataSource>(mDataSource));
    mCurrentIndex = 0;
    mBpmTriggerTime = 60.0f / mBpm;
    mActiveQuakes.clear();
    mIsCapturing = false;
}

// ----------------------------------------------------------------
//
void Tectonic::resize()
{
    Scene::resize();
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
            mBpmTapTimer += dt;
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
            
            // time to keep dot on screen proportional to magnitude
            const float durationMagnitudeMultiplier = (mDataSource == DATASOURCE_NUKES) ? 0.00025f : 0.25f;
            float duration = (60.0f / mBpm + durationMagnitudeMultiplier*mQuakes[mCurrentIndex]->getEventData()->getMag()) / mTimeScale;
            if( mIsCapturing ) duration *= kCaptureFramerate / mApp->getAverageFps();
            
            mQuakes[mCurrentIndex]->trigger(duration);
            mActiveQuakes.push_back( mQuakes[mCurrentIndex] );
            console() << mCurrentIndex << ": " << mQuakes[mCurrentIndex]->getEventData()->toString() << std::endl;
            
            if( mSendToBinned )
            {
                Binned* binnedScene = static_cast<Binned*>(mApp->getScene("binned"));
                
                if( binnedScene && binnedScene->isRunning() )
                {
                    const float radMult = 20.0f;
                    const Vec3f& pos = mQuakes[mCurrentIndex]->getPosition();
                    const QuakeEvent* event = mQuakes[mCurrentIndex]->getEventData();
                    const float forceMult = 0.5f;
                    binnedScene->addRepulsionForce(Vec2f( pos.x, pos.y ), 
                                                   20.0f + event->getMag()*radMult, 
                                                   100.0f + event->getDepth()*forceMult);
                }
            }
            
//            if( mGenerateAudio )
//            {
//                const float freqMultiplier = 1.0f - (mQuakes[mCurrentIndex]->getEventData()->getMag() / 50000.0f);
//                uint32_t freq = 15 + freqMultiplier * 500;
//                audio::Output::play( audio::createCallback( new SineWave( freq, duration ), &SineWave::getData, true ) );
//            }
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

bool Tectonic::bpmTap()
{
    if( mBpmTapTimer < 2.0f )
    {
        mBpmAverage = (mBpmAverage + (60.0f / mBpmTapTimer)) / 2.0f;
        mBpm = mBpmAverage;
        console() << "[tectonic] TAP " << mBpmTapTimer << ". " << "Average BPM = " << mBpm << std::endl;
        triggerNextQuake();
    }
    else
    {
        mBpmAverage = mBpm;
    }
    mBpmTapTimer = 0.0f;
    return false;
}

// ----------------------------------------------------------------
//
void Tectonic::draw()
{
    gl::pushMatrices();
    gl::setMatricesWindow( mApp->getViewportWidth(), mApp->getViewportHeight() );
    
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
    gl::color( 1.0f, 1.0f, 1.0f, mEarthAlpha );
    //gl::draw( mEarthDiffuse, Rectf( 0, 0, mApp->getViewportWidth(), mApp->getViewportHeight() ) );
    const float textureWidth = mEarthDiffuse.getCleanWidth();
    const float screenWidth = mApp->getViewportWidth();
    const float screenHeight = mApp->getViewportHeight();
    const float textureOffset = ((float)(mLongitudeOffsetDegrees)/360.0f) * textureWidth;
    
    gl::draw( mEarthDiffuse, Area( -textureOffset, 0, textureWidth-textureOffset, mEarthDiffuse.getCleanHeight() ), Rectf( 0, 0, screenWidth, screenHeight ) );
}

// ----------------------------------------------------------------
//
void Tectonic::drawQuakes()
{
    const float ringsMultiplier = 0.01f;
    const float radiusMultiplier = (mDataSource == DATASOURCE_NUKES) ? 0.005f : 0.01f;
    gl::disableDepthRead();
    for(QuakeList::iterator it = mActiveQuakes.begin(); 
        it != mActiveQuakes.end();
        ++it)
    {
        Color color = mMarkerColor;
        if( mDataSource == DATASOURCE_NUKES && mColorNukesByType )
        {
            switch ((*it)->getEventData()->getType()) {
                case 0:// Atmospheric
                    color = Color(1.0f,0.5f,0.0f);
                    break;
                    
                case 1:// Underground
                    color = Color(1.0f,0.0f,0.0f);
                    break;
                    
                case 2:// Underwater
                    color = Color(0.0f,0.0f,1.0f);
                    break;
                    
                default:
                    break;
            }
        }
        (*it)->draw(color, ringsMultiplier, radiusMultiplier);
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
    //gl::setMatricesWindow( mApp->getViewportWidth(), mApp->getViewportHeight() );

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

// ----------------------------------------------------------------
//
bool Tectonic::changeDataSource()
{
    reset();
    return false;
}

// ----------------------------------------------------------------
//

//SineWave::SineWave( uint32_t freq, float duration )
//: mFreq( freq ), mDuration( duration )
//{
//}
//
//// ----------------------------------------------------------------
////
//void SineWave::getData( uint64_t inSampleOffset, uint32_t inSampleCount, ci::audio::Buffer32f *ioBuffer )
//{
//	if( ( inSampleOffset / 44100.0f ) > mDuration ) {
//		ioBuffer->mDataByteSize = 0;
//		return;
//	}
//	
//	uint64_t idx = inSampleOffset;
//	
//	for( int  i = 0; i < inSampleCount; i++ ) {
//		
//		float val = ci::math<float>::sin( idx * ( mFreq / 44100.0f ) * 2.0f * M_PI );
//		
//		ioBuffer->mData[i*ioBuffer->mNumberChannels] = val;
//		ioBuffer->mData[i*ioBuffer->mNumberChannels + 1] = val;
//		idx++;
//	}
//}

