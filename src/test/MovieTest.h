/*
 *  MovieTest.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __MOVIETEST_H__
#define __MOVIETEST_H__

#include <vector>
#include "cinder/Cinder.h"
#include "cinder/audio/Input.h"
#include "Scene.h"
#include "MoviePlayer.h"

//
// Audio input tests
//
class MovieTest : public Scene
{
public:
    MovieTest() {}
    virtual ~MovieTest() {}
    
    // inherited from Scene
    void setup();
    //void setupParams(params::InterfaceGl& params);
    //void reset();
    void update(double dt);
    void draw();
    bool handleKeyDown(const KeyEvent& keyEvent);
    
private:
    MoviePlayer mMoviePlayer1;
    
    
};

#endif // __MOVIETEST_H__
