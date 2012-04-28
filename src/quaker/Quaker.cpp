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
    
    reset();
}

// ----------------------------------------------------------------
//
void Quaker::initQuakes()
{
    clearQuakes();
    
    mData->parseData("http://earthquake.usgs.gov/earthquakes/catalogs/7day-M2.5.xml");
    
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
}

// ----------------------------------------------------------------
//
void Quaker::update(double dt)
{
    for(QuakeList::iterator it = mQuakes.begin(); 
        it != mQuakes.end();
        ++it)
    {
        (*it)->update(dt);
    }
    
    // last
    Scene::update(dt);
}

// ----------------------------------------------------------------
//
void Quaker::draw()
{
    gl::pushMatrices();
    gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
    
    gl::enableAlphaBlending();
    
    drawEarthMap();
    
    gl::disableDepthRead();
    for(QuakeList::iterator it = mQuakes.begin(); 
        it != mQuakes.end();
        ++it)
    {
        (*it)->draw();
    }
    
    gl::popMatrices();
}

// ----------------------------------------------------------------
//
void Quaker::drawEarthMap()
{
    gl::color( 1.0f, 1.0f, 1.0f, 1.0f );
    gl::draw( mEarthDiffuse, Rectf( 0, 0, mApp->getViewportWidth(), mApp->getViewportHeight() ) );
}

// ----------------------------------------------------------------
//
void Quaker::drawDebug()
{
    gl::pushMatrices();
    gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );

    gl::popMatrices();
}
