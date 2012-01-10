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

//MovieTest::MovieTest()
//{
//}
//
//MovieTest::~MovieTest()
//{
//}

void MovieTest::setup()
{
    mMoviePlayer1.setup();
}

void MovieTest::update(double dt)
{
    mMoviePlayer1.update(dt);
}

void MovieTest::draw()
{
    gl::pushMatrices();
    
    //CameraOrtho textCam(0.0f, app::getWindowWidth(), app::getWindowHeight(), 0.0f, 0.0f, 10.f);
    CameraPersp cam(app::getWindowWidth(), app::getWindowHeight(), 60.0f);
    gl::setMatrices(cam);
    
    mMoviePlayer1.draw();
    
    gl::popMatrices();
}

bool MovieTest::handleKeyDown(const KeyEvent& keyEvent)
{
    bool handled = true;
    
    switch (keyEvent.getChar()) 
    {
        case 'o':
            mMoviePlayer1.loadMoviePrompt();
            break;
        default:
            handled = false;
            break;
    }
    
    return handled;    
}
