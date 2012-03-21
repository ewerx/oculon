/*
 *  MovieTest.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "OculonApp.h"//TODO: fix this dependency
#include "MovieTest.h"
#include "cinder/Utilities.h"
#include <boost/format.hpp>


using namespace ci;

MovieTest::MovieTest()
: mActiveMovie(0)
, mCycleDuration(0.0f)
, mCycleTimer(0.0f)
{
}

MovieTest::~MovieTest()
{
    for( int i = 0; i < NUM_MOVIES; ++i )
    {
        delete mMoviePlayers[i];
    }
}

void MovieTest::setup()
{
    fs::path filePath;
    boost::format filenameFormat("~/Desktop/oculon%03d.mov");
    string pathString;
    
    for( int i = 0; i < NUM_MOVIES; ++i )
    {
        mMoviePlayers[i] = new MoviePlayer(this);
    }
    
    for( int i=0; i < NUM_MOVIES; ++i )
    {
        mMoviePlayers[i]->setup();
        mMoviePlayers[i]->setSize( app::getWindowWidth(), app::getWindowHeight() );
        
        pathString = str( filenameFormat % i );
        filePath = expandPath( fs::path(pathString) );
        mMoviePlayers[i]->loadMovieFile( filePath );
        
        mMoviePlayers[i]->setColor( ColorA( 1.0f-i*0.3f, i*0.3f, 1.0f-i*0.3f, 1.0f ) );
        //mMoviePlayers[i]->loadMovieFile( fs::path("/Users/ehsan/Downloads/NASA's Alien Anomalies caught on film - A compilation of stunning UFO footage from NASA's archives [H.264 360p].mp4") );
        //mMoviePlayers[i]->getMovie().stop();
    }
    
    mCycleDuration = 60.0f/125.0f; // 125 bpm
    
}

void MovieTest::setActive(bool active)
{
    Scene::setActive(active);
    
    for( int i=0; i < NUM_MOVIES; ++i )
    {
        if( mMoviePlayers[i]->isPlaying() && !active )
        {
            mMoviePlayers[i]->getMovie().stop();
        }
        else if( !mMoviePlayers[i]->isPlaying() && active )
        {
            mMoviePlayers[i]->getMovie().play();
        }
    }
}

void MovieTest::update(double dt)
{
    mCycleTimer += dt;
    if( mCycleTimer >= mCycleDuration )
    {
        mCycleTimer = 0.0f;
        ++mActiveMovie;
        if( mActiveMovie == NUM_MOVIES )
        {
            mActiveMovie = 0;
        }
    }
    
    if( mActiveMovie >= 0 && mActiveMovie < NUM_MOVIES )
    {
        mMoviePlayers[mActiveMovie]->update(dt);
    }
    else
    {
        assert(false && "active movie out of range");
    }
}

void MovieTest::draw()
{
    gl::pushMatrices();
    
    //CameraOrtho cam(0.0f, app::getWindowWidth(), app::getWindowHeight(), 0.0f, 0.0f, 1.0f);
    //CameraPersp cam(app::getWindowWidth(), app::getWindowHeight(), 60.0f);
    //gl::setMatrices(cam);
    gl::setMatricesWindowPersp(app::getWindowWidth(), app::getWindowHeight(), 60.0f, -1.0f, 0.0f);
    
    gl::enableDepthRead(false);
    //gl::enableDepthWrite(false);
    gl::enableAdditiveBlending();
    
    if( mActiveMovie >= 0 && mActiveMovie < NUM_MOVIES )
    {
        mMoviePlayers[mActiveMovie]->draw();
    }
    else
    {
        assert(false && "active movie out of range");
    }
    
    gl::enableAlphaBlending();
    gl::enableDepthRead(true);
    //gl::enableDepthWrite(true);
    
    gl::popMatrices();
}

bool MovieTest::handleKeyDown(const KeyEvent& keyEvent)
{
    bool handled = true;
    
    switch (keyEvent.getChar()) 
    {
            /*
        case 'o':
            mMoviePlayer1.loadMoviePrompt();
            break;
             */
        case 'b':
            mMoviePlayers[0]->getMovie().stepForward();
            break;
        default:
            handled = false;
            break;
    }
    
    return handled;    
}
