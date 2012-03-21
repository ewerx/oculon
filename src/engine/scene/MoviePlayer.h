/*
 *  MoviePlayer.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-16.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __MOVIEPLAYER_H__
#define __MOVIEPLAYER_H__

#include "cinder/app/AppBasic.h"
#include "cinder/qtime/QuickTime.h"
#include "cinder/Surface.h"
#include "Entity.h"

using namespace ci;

class MoviePlayer : public Entity<float>
{
public:
    MoviePlayer(Scene* scene);
    ~MoviePlayer();
    
    // inherited from Entity
    void setup();
    void shutdown();
    void update(double dt);
    void draw();
    
	void loadMovieUrl( const std::string &urlString );
	void loadMovieFile( const fs::path &path );
    void loadMoviePrompt();
    
    bool isPlaying() const      { return mState == STATE_READY; }
    qtime::MovieGl getMovie()   { return mMovie; }
    
    void setSize( const float width, const float height )   { mWidth = width; mHeight = height; }
    void setColor( const ColorA& color )                    { mColor = color; }
    
protected:
    void setActiveMovie( qtime::MovieGl movie );
    void drawFFT( const qtime::MovieBase &movie, float x, float y, float width, float height );
    
private:
    fs::path mPath;
    qtime::MovieGl mMovie; // built-in smart pointer
    qtime::MovieLoader mLoadingMovie; // built-in smart pointer 
    
    float mWidth;
    float mHeight;
    
    bool mLoop;
    bool mPalindrome;
    
    ColorA mColor;
    
    enum eState
    {
        STATE_INVALID,
        STATE_LOADING,
        STATE_READY,
    };
    eState mState;
};

#endif