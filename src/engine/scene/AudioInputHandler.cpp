//
//  AudioInputHandler.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2013-05-17.
//
//

#include "AudioInputHandler.h"
#include "Interface.h"
#include "Scene.h"
#include "cinder/Rand.h"

using namespace std;

AudioInputHandler::AudioInputHandler()
{
    
}

AudioInputHandler::~AudioInputHandler()
{
}

void AudioInputHandler::setup(const Scene *scene, bool fboEnabled)
{
    mScene = scene;
    
    // SIGNAL
    mRandomSignal       = true;
    mRandomEveryFrame   = true;
    mRandomSeed         = 1234;
    mLinearScale        = false;
    
    // FALLOFF
    mFalloffTime        = 0.32f;
    mFalloffMode        = FALLOFF_OUTQUAD;
    mFalloffByFreq      = true;

    // FBO
    mAudioFboDim        = 23; // 512 bands
    mAudioFboEnabled    = fboEnabled;
    if (mAudioFboEnabled)
    {
        mAudioFboSize   = Vec2f( mAudioFboDim, mAudioFboDim );
        mAudioFboBounds = Area( 0, 0, mAudioFboDim, mAudioFboDim );
        gl::Fbo::Format audioFboFormat;
        audioFboFormat.setColorInternalFormat( GL_RGB32F_ARB );
        mAudioFbo       = gl::Fbo( mAudioFboDim, mAudioFboDim, audioFboFormat );
    }
}

void AudioInputHandler::setupInterface( Interface* interface )
{
    interface->gui()->addColumn();
    interface->gui()->addLabel("audio");
    interface->addParam(CreateBoolParam( "random", &mRandomSignal )
                         .oscReceiver(mScene->getName(),"audio/random"));
    interface->addParam(CreateBoolParam( "randomize", &mRandomEveryFrame )
                        .oscReceiver(mScene->getName(),"audio/randomize"));
    interface->addParam(CreateBoolParam( "linear", &mLinearScale )
                        .oscReceiver(mScene->getName(),"audio/linear"));
    
    interface->addParam(CreateFloatParam( "falloff", &mFalloffTime )
                        .maxValue(5.0f)
                        .oscReceiver(mScene->getName(),"audio/falloff"));
    interface->addParam(CreateBoolParam( "freq_falloff", &mFalloffByFreq )
                        .oscReceiver(mScene->getName(),"audio/freq_falloff"));
    
    vector<string> falloffModeNames;
#define AUDIO_FALLOFF_MODE_ENTRY( nam, enm ) \
falloffModeNames.push_back(nam);
    AUDIO_FALLOFF_MODE_TUPLE
#undef  AUDIO_FALLOFF_MODE_ENTRY
interface->addEnum(CreateEnumParam( "falloff_mode", (int*)(&mFalloffMode) )
                        .maxValue(FALLOFFMODE_COUNT)
                        .oscReceiver(mScene->getName(), "audio/falloff_mode")
                        .isVertical(), falloffModeNames);

}

void AudioInputHandler::update(double dt, AudioInput& audioInput)
{
    int32_t dataSize = audioInput.getFft()->getBinSize();
    const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
    if( mFftFalloff.size() == 0 )
    {
        for( int i=0; i< dataSize; ++i )
        {
            mFftFalloff.push_back( tFftValue( i, fftLogData[i].y ) );
        }
    }
    
    int32_t row = 0;
    
    if (mRandomEveryFrame) {
        mRandomSeed = Rand::randInt();
    }
    Rand randIndex(mRandomSeed);

	Surface32f fftSurface;
    if (mAudioFboEnabled)
    {
        fftSurface = Surface32f( mAudioFbo.getTexture() );
    }
    else
    {
        const float size = mAudioFboDim*mAudioFboDim;
        fftSurface = Surface32f( size, size, true );
    }
    
	Surface32f::Iter it = fftSurface.getIter();
    int32_t index = 0;
	while( it.line() )
    {
        //int32_t index = row * mAudioFboDim;
		while( it.pixel() && index < dataSize )
        {
            int32_t bandIndex = mRandomSignal ? randIndex.nextInt(dataSize) : index;
            
            float falloff = mFalloffByFreq ? (mFalloffTime * (1.0f - bandIndex / dataSize)) : mFalloffTime;
            
            float value = mLinearScale ? (fftLogData[bandIndex].y * (1+bandIndex)) : fftLogData[bandIndex].y;
            if (mScene)
            {
                value *= mScene->getGain();
            }
            
            if (value > mFftFalloff[index].mValue)
            {
                mFftFalloff[index].mFalling = false;
                mFftFalloff[index].mBandIndex = bandIndex;
                // fade in
                timeline().apply( &mFftFalloff[index].mValue, value, mFalloffTime*0.1f, EaseNone() );
                //timeline().appendTo(&mFftFalloff[index].mValue, 0.0f, falloff, getFalloffFunction() );
            } else if (!mFftFalloff[index].mFalling && value < (mFftFalloff[index].mValue*0.5f)) {
                // fade out
                mFftFalloff[index].mFalling = true;
                timeline().apply( &mFftFalloff[index].mValue, 0.0f, falloff, getFalloffFunction() );
            }
            
			it.r() = mFftFalloff[index].mValue();
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
	
    if (mAudioFboEnabled) {
        gl::Texture fftTexture( fftSurface );
        mAudioFbo.bindFramebuffer();
        gl::setMatricesWindow( mAudioFboSize, false );
        gl::setViewport( mAudioFboBounds );
        gl::draw( fftTexture );
        mAudioFbo.unbindFramebuffer();
    }
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


