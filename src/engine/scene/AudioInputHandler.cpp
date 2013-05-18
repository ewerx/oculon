//
//  AudioInputHandler.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2013-05-17.
//
//

#include "AudioInputHandler.h"
#include "cinder/Rand.h"

AudioInputHandler::AudioInputHandler()
{
    
}

AudioInputHandler::~AudioInputHandler()
{
}

void AudioInputHandler::setup()
{
    // FALLOFF
    mFalloffTime = 0.32f;
    mFalloffMode = FALLOFF_LINEAR;

    // FBO
    mAudioFboDim    = 16; // 256 bands
    mAudioFboSize   = Vec2f( mAudioFboDim, mAudioFboDim );
    mAudioFboBounds = Area( 0, 0, mAudioFboDim, mAudioFboDim );
    gl::Fbo::Format audioFboFormat;
	audioFboFormat.setColorInternalFormat( GL_RGB32F_ARB );
    mAudioFbo       = gl::Fbo( mAudioFboDim, mAudioFboDim, audioFboFormat );
}

void AudioInputHandler::update(double dt, AudioInput& audioInput)
{
    int32_t dataSize = audioInput.getFft()->getBinSize();
    const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
    if( mFftFalloff.size() == 0 )
    {
        for( int i=0; i< dataSize; ++i )
        {
            mFftFalloff.push_back( fftLogData[i].y );
        }
    }
    
    int32_t row = 0;//mAudioRowShift;
    
    //Rand randIndex(0);
	Surface32f fftSurface( mAudioFbo.getTexture() );
	Surface32f::Iter it = fftSurface.getIter();
    int32_t index = 0;
	while( it.line() )
    {
        //int32_t index = row * mAudioFboDim;
		while( it.pixel() && index < dataSize )
        {
            int32_t bandIndex = Rand::randInt(dataSize);//randIndex.nextInt(dataSize);
            if (fftLogData[bandIndex].y > mFftFalloff[index])
            {
                //mFftFalloff[index] = fftLogData[bandIndex].y;
                timeline().apply( &mFftFalloff[index], fftLogData[bandIndex].y, mFalloffTime/2.0f, getReverseFalloffFunction() );
                timeline().appendTo(&mFftFalloff[index], 0.0f, mFalloffTime, getReverseFalloffFunction() );
                //timeline().apply( &mFftFalloff[index], 0.0f, mFalloff, getFalloffFunction() );
            } else if (fftLogData[bandIndex].y < mFftFalloff[index]) {
                timeline().apply( &mFftFalloff[index], 0.0f, mFalloffTime, getFalloffFunction() );
            }
            
			it.r() = mFftFalloff[index]();
            it.g() = 0.0f; // UNUSED
			it.b() = 0.0f; // UNUSED
			it.a() = 1.0f; // UNUSED
            
            ++index;
		}
        
        ++row;
        if (row >= mAudioFboDim)
        {
            row = 0;
        }
	}
    
//    if (mAudioRowShiftTime >= mAudioRowShiftDelay)
//    {
//        mAudioRowShiftTime = 0.0f;
//        ++mAudioRowShift;
//        if (mAudioRowShift >= mAudioFboDim) {
//            mAudioRowShift = 0;
//        }
//    }
	
	gl::Texture fftTexture( fftSurface );
	mAudioFbo.bindFramebuffer();
	gl::setMatricesWindow( mAudioFboSize, false );
	gl::setViewport( mAudioFboBounds );
	gl::draw( fftTexture );
	mAudioFbo.unbindFramebuffer();
}

void AudioInputHandler::drawDebug(const Vec2f& size)
{
    gl::enable( GL_TEXTURE_2D );
    gl::setMatricesWindow( getWindowSize() );
    
    Rectf preview( 100.0f, size.y - 200.0f, 180.0f, size.y - 120.0f );
    gl::draw( mAudioFbo.getTexture(), mAudioFbo.getBounds(), preview );
    
    //mAudioFbo.bindTexture();
    //TODO: make utility func for making rects with origin/size
    //gl::drawSolidRect( Rectf( 100.0f, mApp->getWindowHeight() - 120.0f, 180.0f, mApp->getWindowHeight() - 40.0f ) );
    
    gl::disable( GL_TEXTURE_2D );
}

#pragma mark - Falloff Fucntions

AudioInputHandler::tEaseFn AudioInputHandler::getFalloffFunction()
{
    switch( mFalloffMode )
    {
        case FALLOFF_LINEAR: return EaseNone();
        case FALLOFF_OUTQUAD: return EaseOutQuad();
        case FALLOFF_OUTEXPO: return EaseOutExpo();
        case FALLOFF_OUTBACK: return EaseOutBack();
        case FALLOFF_OUTBOUNCE: return EaseOutBounce();
        case FALLOFF_OUTINEXPO: return EaseOutInExpo();
        case FALLOFF_OUTINBACK: return EaseOutInBack();
            
        default: return EaseNone();
    }
}

AudioInputHandler::tEaseFn AudioInputHandler::getReverseFalloffFunction()
{
    switch( mFalloffMode )
    {
        case FALLOFF_LINEAR: return EaseNone();
        case FALLOFF_OUTQUAD: return EaseInQuad();
        case FALLOFF_OUTEXPO: return EaseInExpo();
        case FALLOFF_OUTBACK: return EaseInBack();
        case FALLOFF_OUTBOUNCE: return EaseInBounce();
        case FALLOFF_OUTINEXPO: return EaseInOutExpo();
        case FALLOFF_OUTINBACK: return EaseInOutBack();
            
        default: return EaseNone();
    }
}


