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
	void loadMovieFile( const ci::fs::path &path );
    void loadMoviePrompt();
    
    bool isPlaying() const          { return mState == STATE_READY; }
    ci::qtime::MovieGl getMovie()   { return mMovie; }
    
    void setSize( const float width, const float height )       { mWidth = width; mHeight = height; }
    void setColor( const ci::ColorA& color )                    { mColor = color; }
    
protected:
    void setActiveMovie( ci::qtime::MovieGl movie );
    void drawFFT( const ci::qtime::MovieBase &movie, float x, float y, float width, float height );
    
private:
    ci::fs::path mPath;
    ci::qtime::MovieGl mMovie; // built-in smart pointer
    ci::qtime::MovieLoader mLoadingMovie; // built-in smart pointer
    
    float mWidth;
    float mHeight;
    
    bool mLoop;
    bool mPalindrome;
    
    ci::ColorA mColor;
    
    enum eState
    {
        STATE_INVALID,
        STATE_LOADING,
        STATE_READY,
    };
    eState mState;
};

#endif