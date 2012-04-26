/*
 *  Quaker.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "OculonApp.h"//TODO: fix this dependency
#include "AudioInput.h"
#include "Quaker.h"
#include "KissFFT.h"
#include "cinder/Rand.h"
#include "Interface.h"

using namespace ci;
using namespace ci::app;

Quaker::Quaker()
: Scene("Quaker")
{
    mData = new USGSQuakeData();
}

Quaker::~Quaker()
{
    delete mData;
}

// ----------------------------------------------------------------
//
void Quaker::setup()
{
    mData->parseData("http://earthquake.usgs.gov/earthquakes/catalogs/7day-M2.5.xml");
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
    
}

// ----------------------------------------------------------------
//
void Quaker::update(double dt)
{
    Scene::update(dt);
}

// ----------------------------------------------------------------
//
void Quaker::draw()
{
    gl::pushMatrices();
    gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
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
