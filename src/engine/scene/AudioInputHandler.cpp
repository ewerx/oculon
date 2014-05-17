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
#include <sstream>

using namespace ci;
using namespace ci::app;
using namespace std;
using std::ostringstream;

AudioInputHandler::AudioInputHandler()
{
    
}

AudioInputHandler::~AudioInputHandler()
{
}

void AudioInputHandler::setup(bool fboEnabled)
{
    // SOURCE
    mInputSource        = SOURCE_AUDIO;
    mGain               = 1.0f;
    
    // DISTRIBUTION
    mRandomSignal       = false;
    mRandomEveryFrame   = false;
    mRandomSeed         = 1234;
    mLinearScale        = false;
    
    // FALLOFF
    mFalloffTime        = 0.32f;
    mFalloffMode        = FALLOFF_OUTQUAD;
    mFalloffByFreq      = true;
    
    // FILTER
    mLowPassFilter      = 0.25f;
    mHighPassFilter     = 0.75f;
    mAvgVolume[BAND_LOW].mValue      = 0.0f;
    mAvgVolume[BAND_MID].mValue      = 0.0f;
    mAvgVolume[BAND_HIGH].mValue     = 0.0f;

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
        
        mAudioFbo.bindFramebuffer();
        gl::setViewport( mAudioFbo.getBounds() );
        gl::clear();
        mAudioFbo.unbindFramebuffer();
    }
    
    mOSCFalloff = false;
}

void AudioInputHandler::setupInterface( Interface* interface, const std::string &name )
{
    interface->gui()->addColumn();
    string label = name + "/audio";
    interface->gui()->addLabel(label);
    
    interface->addParam(CreateFloatParam("gain", &mGain)
                        .maxValue(50.0f)
                        .oscReceiver(name).sendFeedback());
    
//    vector<string> sourceNames;
//#define AUDIO_SOURCE_ENTRY( nam, enm ) \
//sourceNames.push_back(nam);
//    AUDIO_SOURCE_TUPLE
//#undef  AUDIO_SOURCE_ENTRY
//    interface->addEnum(CreateEnumParam( "input_source", (int*)(&mInputSource) )
//                       .maxValue(AUDIO_SOURCE_COUNT)
//                       .oscReceiver(name, "audio/input_source")
//                       .isVertical(), sourceNames);
//    interface->addParam(CreateBoolParam( "OSCFalloff", &mOSCFalloff ));

    interface->addParam(CreateFloatParam( "LPF", &mLowPassFilter ));
    interface->addParam(CreateFloatParam( "HPF", &mHighPassFilter ));
    interface->gui()->addSeparator();
    // these are just to show a little equalizer
    interface->addParam(CreateFloatParam( "Avg Vol: Low", mAvgVolume[BAND_LOW].mValue.ptr() ));
    interface->addParam(CreateFloatParam( "Avg Vol: Mid", mAvgVolume[BAND_MID].mValue.ptr() ));
    interface->addParam(CreateFloatParam( "Avg Vol: High", mAvgVolume[BAND_HIGH].mValue.ptr() ));
    
    interface->gui()->addLabel("distribution");
    if (mAudioFboEnabled)
    {
        interface->gui()->addParam("audiodata", &mAudioFbo.getTexture());
    }
//    interface->addParam(CreateBoolParam( "texture", &mAudioFboEnabled )
//                        .oscReceiver(name,"audio/texture"));
    interface->addParam(CreateBoolParam( "random", &mRandomSignal )
                         .oscReceiver(name,"audio/random"));
    interface->addParam(CreateBoolParam( "randomize", &mRandomEveryFrame )
                        .oscReceiver(name,"audio/randomize"));
    interface->addParam(CreateBoolParam( "linear", &mLinearScale )
                        .oscReceiver(name,"audio/linear"));
    
    interface->gui()->addLabel("falloff");
    interface->addParam(CreateFloatParam( "falloff", &mFalloffTime )
                        .maxValue(5.0f)
                        .oscReceiver(name,"audio/falloff"));
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

vector<string> AudioInputHandler::getBandNames()
{
    vector<string> names;
    names.push_back("low");
    names.push_back("mid");
    names.push_back("high");
    names.push_back("none");
    
    return names;
}

void AudioInputHandler::update(double dt, AudioInput& audioInput)
{
    if (mInputSource > SOURCE_AUDIO)
    {
        float values[BAND_COUNT];
        values[BAND_LOW] = audioInput.getLowLevelForLiveTrack(mInputSource-1);
        values[BAND_MID] = audioInput.getMidLevelForLiveTrack(mInputSource-1);
        values[BAND_HIGH] = audioInput.getHighLevelForLiveTrack(mInputSource-1);
        console() << values[BAND_LOW] << std::endl;
        if(mOSCFalloff)
        {
            for (int i = 0; i < BAND_COUNT; ++i) {
                const float value = values[i];
                if (value > mAvgVolume[i].mValue)
                {
                    mAvgVolume[i].mFalling = false;
                    // fade in (2ms)
                    timeline().apply( &mAvgVolume[i].mValue, value, 0.002f, EaseNone() );
                    //mAvgVolume[i].mValue = value;
                } else if (!mAvgVolume[i].mFalling && value < (mAvgVolume[i].mValue*0.5f)) {
                    // fade out
                    mAvgVolume[i].mFalling = true;
                    timeline().apply( &mAvgVolume[i].mValue, 0.0f, mFalloffTime, getFalloffFunction() );
                }
            }
        }
        else
        {
            mAvgVolume[BAND_LOW].mValue = values[BAND_LOW] * mGain;
            mAvgVolume[BAND_MID].mValue = values[BAND_MID] * mGain;
            mAvgVolume[BAND_HIGH].mValue = values[BAND_HIGH] * mGain;
        }
        
        return;
    }
    
    vector<float> magSpectrum = audioInput.getMagSpectrum();
    
    //TODO: calc average in the main loop
    //int lowPassBand = (int)(mLowPassFilter * KISS_DEFAULT_DATASIZE);
    //int highPassBand = (int)(mHighPassFilter * KISS_DEFAULT_DATASIZE);
    
    if( mFftFalloff.size() == 0 )
    {
        for( int band = 0; band < magSpectrum.size(); ++band )
        {
            mFftFalloff.push_back( tFftValue( band, 0.0f ) );
        }
    }
    else
    {
        if (mRandomEveryFrame) {
            mRandomSeed = Rand::randInt();
        }
        Rand randIndex(mRandomSeed);
        for( int index=0; index< magSpectrum.size(); ++index )
        {
            int32_t bandIndex = mRandomSignal ? randIndex.nextInt(magSpectrum.size()) : index;
            
            float falloff = mFalloffByFreq ? (mFalloffTime * (1.0f - bandIndex / magSpectrum.size())) : mFalloffTime;
            
            float value = magSpectrum[bandIndex];
            value *= mGain;
            
            if (value > mFftFalloff[index].mValue)
            {
                mFftFalloff[index].mFalling = false;
                mFftFalloff[index].mBandIndex = bandIndex;
                // fade in (20ms)
                timeline().apply( &mFftFalloff[index].mValue, value, 0.002f, EaseNone() );
                //mFftFalloff[index].mValue = value;
            } else if (!mFftFalloff[index].mFalling && value < (mFftFalloff[index].mValue*0.5f)) {
                // fade out
                mFftFalloff[index].mFalling = true;
                timeline().apply( &mFftFalloff[index].mValue, 0.0f, falloff, getFalloffFunction() );
            }
        }
    }
    
    if (mAudioFboEnabled)
    {
        audio::Buffer buffer = audioInput.getBuffer();
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
                    if (col < buffer.getNumChannels())
                    {
                        float value = buffer.getChannel(0)[col] * mGain;
                        it.r() = 0.5f + 0.5f * value;
                        it.g() = 0.5f + 0.5f * value;
                        it.b() = 0.5f + 0.5f * value;
                        it.a() = 1.0f;
                    }
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
        gl::color(ColorA::white());
        gl::clear();
        gl::draw( fftTexture );
        mAudioFbo.unbindFramebuffer();
        gl::popMatrices();
    }
    
    // calculate avg volumes for low/mid/high for quick access and visual display
    mAvgVolume[BAND_LOW].mValue = getAverageVolumeByFrequencyRange(0.0f, mLowPassFilter);
    //HACKHACK: need to figure out why mid/high are always so low compared to bass
    mAvgVolume[BAND_MID].mValue = getAverageVolumeByFrequencyRange(mLowPassFilter, mHighPassFilter) * 10.0f;
    mAvgVolume[BAND_HIGH].mValue = getAverageVolumeByFrequencyRange(mHighPassFilter, 1.0f) * 5.0f;
}

void AudioInputHandler::drawDebug(const Vec2f& windowSize)
{
    if (mAudioFboEnabled)
    {
        gl::pushMatrices();
        glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
        gl::enable( GL_TEXTURE_2D );
        gl::setMatricesWindow( getWindowSize() );
        
        const float size = 80.0f;
        const float paddingX = 20.0f;
        const float paddingY = 240.0f;
        Rectf preview( windowSize.x - (size+paddingX), windowSize.y - (size+paddingY), windowSize.x-paddingX, windowSize.y - paddingY );
        gl::draw( mAudioFbo.getTexture(), preview );
        
        //mAudioFbo.bindTexture();
        //TODO: make utility func for making rects with origin/size
        //gl::drawSolidRect( Rectf( 100.0f, mApp->getWindowHeight() - 120.0f, 180.0f, mApp->getWindowHeight() - 40.0f ) );
        
        glPopAttrib();
        gl::popMatrices();
    }
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
//        case FALLOFF_OUTINEXPO: return EaseOutInExpo();
//        case FALLOFF_OUTINBACK: return EaseOutInBack();
            
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
//        case FALLOFF_OUTINEXPO: return EaseInOutExpo();
//        case FALLOFF_OUTINBACK: return EaseInOutBack();
            
        default: return EaseNone();
    }
}

#pragma mark - Easy Accessors

float AudioInputHandler::getAverageVolumeByFrequencyRange(const float minRatio, const float maxRatio)
{
    return getAverageVolumeByFrequencyRange( (int)(minRatio * KISS_DEFAULT_DATASIZE), (int)(maxRatio * KISS_DEFAULT_DATASIZE) );
}

float AudioInputHandler::getAverageVolumeByFrequencyRange(const int minBand /*=0*/, const int maxBand /*=256*/)
{
    float amplitude = 0.0f;
    
    int minIndex = math<int>::max( 0, minBand );
    int maxIndex = math<int>::min( mFftFalloff.size(), maxBand );
    
    for (int32_t i = minIndex; i < maxIndex; i++)
    {
        amplitude += mFftFalloff[i].mValue;
    }
    
    // WAT?
    amplitude = amplitude / (float)(maxIndex-minIndex);
    
    return amplitude;
}

