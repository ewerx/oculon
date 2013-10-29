//
//  AudioInputHandler.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2013-05-17.
//
//

#include "AudioInputHandler.h"
#include "Interface.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

AudioInputHandler::AudioInputHandler()
{
    
}

AudioInputHandler::~AudioInputHandler()
{
}

void AudioInputHandler::setup(bool fboEnabled)
{
    
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
    mAudioFboDim        = KISS_DEFAULT_DATASIZE; // 256 bands
    mAudioFboEnabled    = fboEnabled;
    const int audioFboHeight = 2;
    if (mAudioFboEnabled)
    {
        mAudioFboSize   = ci::Vec2f( mAudioFboDim, audioFboHeight );
        mAudioFboBounds = ci::Area( 0, 0, mAudioFboDim, audioFboHeight );
        gl::Fbo::Format audioFboFormat;
        audioFboFormat.setColorInternalFormat( GL_RGB32F_ARB );
        mAudioFbo       = gl::Fbo( mAudioFboDim, audioFboHeight, audioFboFormat );
    }
}

void AudioInputHandler::setupInterface( Interface* interface, const std::string &name )
{
    interface->gui()->addColumn();
    interface->gui()->addLabel("audio");
//    interface->addParam(CreateBoolParam( "texture", &mAudioFboEnabled )
//                        .oscReceiver(name,"audio/texture"));
    interface->addParam(CreateBoolParam( "random", &mRandomSignal )
                         .oscReceiver(name,"audio/random"));
    interface->addParam(CreateBoolParam( "randomize", &mRandomEveryFrame )
                        .oscReceiver(name,"audio/randomize"));
    interface->addParam(CreateBoolParam( "linear", &mLinearScale )
                        .oscReceiver(name,"audio/linear"));
    
    interface->addParam(CreateFloatParam( "falloff", &mFalloffTime )
                        .maxValue(5.0f)
                        .oscReceiver(name,"audio/falloff")
                        .midiInput(1, 2, 22));
    interface->addParam(CreateBoolParam( "freq_falloff", &mFalloffByFreq )
                        .oscReceiver(name,"audio/freq_falloff"));
    
    vector<string> falloffModeNames;
#define AUDIO_FALLOFF_MODE_ENTRY( nam, enm ) \
falloffModeNames.push_back(nam);
    AUDIO_FALLOFF_MODE_TUPLE
#undef  AUDIO_FALLOFF_MODE_ENTRY
interface->addEnum(CreateEnumParam( "falloff_mode", (int*)(&mFalloffMode) )
                        .maxValue(FALLOFFMODE_COUNT)
                        .oscReceiver(name, "audio/falloff_mode")
                        .isVertical(), falloffModeNames);

}

void AudioInputHandler::update(double dt, AudioInput& audioInput, float gain)
{
    if (audioInput.getFft() == NULL)
    {
        return;
    }
    
    int32_t dataSize = audioInput.getFft()->getBinSize();
    const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
    if( mFftFalloff.size() == 0 )
    {
        for( int i=0; i< dataSize; ++i )
        {
            mFftFalloff.push_back( tFftValue( i, fftLogData[i].y ) );
        }
    }
    else
    {
        if (mRandomEveryFrame) {
            mRandomSeed = Rand::randInt();
        }
        Rand randIndex(mRandomSeed);
        for( int index=0; index< dataSize; ++index )
        {
            int32_t bandIndex = mRandomSignal ? randIndex.nextInt(dataSize) : index;
            
            float falloff = mFalloffByFreq ? (mFalloffTime * (1.0f - bandIndex / dataSize)) : mFalloffTime;
            
            float value = mLinearScale ? (fftLogData[bandIndex].y * (1+bandIndex)) : fftLogData[bandIndex].y;
            value *= gain;
            
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
        }
    }
    
    if (mAudioFboEnabled)
    {
        float * timeData = audioInput.getFft()->getData(); // normalized -1 to +1
        Surface32f fftSurface = Surface32f( mAudioFbo.getTexture() );
        
        Surface32f::Iter it = fftSurface.getIter();
        int32_t row = 0;
        while( it.line() )
        {
            int32_t col = 0;
            while( it.pixel() )
            {
                if (row == 0)
                {
                    it.r() = mFftFalloff[col].mValue();
                    it.g() = mFftFalloff[col].mValue();
                    it.b() = mFftFalloff[col].mValue();
                    it.a() = 1.0f;
                }
                else
                {
                    float value = timeData[col] * gain;
                    it.r() = 0.5f + 0.5f * value;
                    it.g() = 0.5f + 0.5f * value;
                    it.b() = 0.5f + 0.5f * value;
                    it.a() = 1.0f;
                }
                
                ++col;
            }
            ++row;
        }
        
        gl::pushMatrices();
        gl::Texture fftTexture( fftSurface );
        mAudioFbo.bindFramebuffer();
        gl::setMatricesWindow( mAudioFboSize, false );
        gl::setViewport( mAudioFboBounds );
        gl::clear();
        gl::draw( fftTexture );
        mAudioFbo.unbindFramebuffer();
        gl::popMatrices();
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


