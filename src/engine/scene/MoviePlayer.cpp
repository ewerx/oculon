#include <iostream>
#include <sstream>
#include "cinder/app/AppBasic.h"
#include "cinder/Surface.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"
#include "cinder/qtime/QuickTime.h"
#include "MoviePlayer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

MoviePlayer::MoviePlayer()
:Entity<float>()
,mWidth(0.0f)
,mHeight(0.0f)
,mLoop(true)
,mPalindrome(false)
,mColor(1.0f,1.0f,1.0f,1.0f)
,mState(STATE_INVALID)
{
}

MoviePlayer::~MoviePlayer()
{
}

void MoviePlayer::setup()
{
}

void MoviePlayer::shutdown()
{
    mMovie.stop();
    mLoadingMovie.reset();
}

void MoviePlayer::setActiveMovie( qtime::MovieGl movie )
{    
    console() << "[movie] Movie started..." << std::endl;
	console() << "\tDimensions:" << movie.getWidth() << " x " << movie.getHeight() << std::endl;
	console() << "\tDuration:  " << movie.getDuration() << " seconds" << std::endl;
	console() << "\tFrames:    " << movie.getNumFrames() << std::endl;
	console() << "\tFramerate: " << movie.getFramerate() << std::endl;
	movie.setLoop( mLoop, mPalindrome );
    movie.setVolume(0.0f);
	
    mMovie = movie;
    
    mMovie.seekToStart();
	mMovie.play();
    
    mState = STATE_READY;
}

void MoviePlayer::loadMovieUrl( const string &urlString )
{
	try 
    {
        mState = STATE_LOADING;
        mLoadingMovie.reset();
		mLoadingMovie = qtime::MovieLoader( Url( urlString ) );
	}
	catch( ... ) 
    {
		console() << "Unable to load the movie from URL: " << urlString << std::endl;
	}
}

void MoviePlayer::loadMovieFile( const fs::path &moviePath )
{
 	try 
    {
        setActiveMovie( qtime::MovieGl( moviePath ) );
        mPath = moviePath;
	}
	catch( ... ) 
    {
		console() << "[movie] Unable to load " << moviePath << std::endl;
		return;
	}
	
	try 
    {
		mMovie.setupMonoFft( 8 );
	}
	catch( qtime::QuickTimeExcFft & ) 
    {
		console() << "[movie] Unable to setup FFT" << std::endl;
	}
}

void MoviePlayer::loadMoviePrompt()
{
    fs::path moviePath = getOpenFilePath();
    if( !moviePath.empty() )
    {
        loadMovieFile( moviePath );
    }
}

void MoviePlayer::update(double /*dt*/)
{
	if( STATE_LOADING == mState )
    {
		try 
        {
			if( mLoadingMovie.checkPlaythroughOk() ) 
            {
				setActiveMovie( mLoadingMovie );
			}
		}
		catch( ... ) 
        {
			console() << "There was an error loading a movie." << std::endl;
			mState = STATE_INVALID;
		}
	}
}

void MoviePlayer::draw()
{
    if( STATE_READY == mState )
    {
        float drawWidth = mMovie.getWidth();
        if( mWidth > 0.0f )
        {
            drawWidth = mWidth;
        }
        float drawHeight = mMovie.getHeight();
        if( mHeight > 0.0f )
        {
            drawHeight = mHeight;
        }
        
        gl::Texture texture = mMovie.getTexture();
        if( texture ) 
        {			
            gl::color( mColor );
            gl::draw( texture, Rectf( mPosition.x, mPosition.y, mPosition.x + drawWidth, mPosition.y + drawHeight ) );
            texture.disable();

            drawFFT( mMovie, mPosition.x, mPosition.y, drawWidth, drawHeight );
        }
    }
}

void MoviePlayer::drawFFT( const qtime::MovieBase &movie, float x, float y, float width, float height )
{
	if( ! movie.getNumFftChannels() )
		return;
	
	float bandWidth = width / movie.getNumFftBands();
	float *fftData = movie.getFftData();
	for( uint32_t band = 0; band < movie.getNumFftBands(); ++band ) {
		float bandHeight = height / 3.0f * fftData[band];
		gl::color( Color( 0.1f, 0.8f, 0.1f ) );
		gl::drawSolidRect( ci::Rectf( x + band * bandWidth, y + height - bandHeight, x + band * bandWidth + bandWidth, y + height ) );
	}
}
