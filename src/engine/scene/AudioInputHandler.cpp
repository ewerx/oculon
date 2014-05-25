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
#include "cinder/audio/Utilities.h"
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
    mLinearScale        = true; // linear or decibel scale
    
    // DISTRIBUTION
    mRandomSignal       = false;
    mRandomize          = false;
    mRandomSeed         = 1234;
    
    // FALLOFF
    mFalloffTime        = 0.32f;
    mFalloffMode        = FALLOFF_OUTQUAD;
    mFalloffByFreq      = false;
    
    // FILTER
    mLowPassFilter      = 0.25f;
    mHighPassFilter     = 0.5f;
    mAvgVolume[BAND_LOW].mValue      = 0.0f;
    mAvgVolume[BAND_MID].mValue      = 0.0f;
    mAvgVolume[BAND_HIGH].mValue     = 0.0f;

    // FBO
    //TODO: audio2 cleanup
    mAudioFboDim        = DEFAULT_FREQ_BIN_SIZE;
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

void AudioInputHandler::setupInterface( Interface* interface, const std::string &name, const int midiChannel, const int midiNote, const int randomizeMidiChannel, const int randomizeMidiNote )
{
    interface->gui()->addColumn();
    string label = name + "/audio";
    interface->gui()->addLabel(label);
    
    interface->addParam(CreateFloatParam("gain", &mGain)
                        .minValue(0.05f)
                        .maxValue(50.0f)
                        .oscReceiver(name).sendFeedback()
                        .midiInput(0, midiChannel, midiNote));
    interface->addParam(CreateBoolParam( "linear", &mLinearScale )
                        .oscReceiver(name,"audio/linear"));
    
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
    interface->addButton(CreateTriggerParam( "randomize", NULL )
                        .oscReceiver(name,"audio/randomize")
                         .midiInput(0, randomizeMidiChannel, randomizeMidiNote))->registerCallback( this, &AudioInputHandler::onRandomize );
    
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

bool AudioInputHandler::onRandomize()
{
    mRandomize = true;
    return true;
}

/*static*/ vector<string> AudioInputHandler::getBandNames()
{
    vector<string> names;
    names.push_back("low");
    names.push_back("mid");
    names.push_back("high");
    names.push_back("none");
    
    return names;
}

float AudioInputHandler::getAverageVolumeByBand(const int bandIndex)
{
    if (bandIndex >= BAND_NONE)
    {
        return 1.0f;
    }
    else
    {
        return mAvgVolume[bandIndex].mValue;
    }
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
    
    //TODO: audio2 cleanup
    //TODO: calc average in the main loop
    //int lowPassBand = (int)(mLowPassFilter * DEFAULT_FREQ_BIN_SIZE);
    //int highPassBand = (int)(mHighPassFilter * DEFAULT_FREQ_BIN_SIZE);
    
    if( mFftFalloff.size() == 0 )
    {
        for( int band = 0; band < magSpectrum.size(); ++band )
        {
            mFftFalloff.push_back( tFftValue( band, 0.0f ) );
        }
    }
    else
    {
        if (mRandomize)
        {
            mRandomSeed = Rand::randInt();
            mRandomize = false;
        }
        Rand randIndex(mRandomSeed);
        for( int index=0; index< magSpectrum.size(); ++index )
        {
            int32_t bandIndex = mRandomSignal ? randIndex.nextInt(magSpectrum.size()) : index;
            
            float falloff = mFalloffByFreq ? (mFalloffTime * (1.0f - bandIndex / magSpectrum.size())) : mFalloffTime;
            
            float value = magSpectrum[bandIndex];
            if (!mLinearScale)
            {
                value = audio::toDecibels(value) / 100.0f;
            }
            value *= mGain;
            
            if (value > mFftFalloff[index].mValue)
            {
                mFftFalloff[index].mFalling = false;
                mFftFalloff[index].mBandIndex = bandIndex;
                // fade in (10ms)
                //timeline().apply( &mFftFalloff[index].mValue, value, 0.001f, EaseNone() );
                mFftFalloff[index].mValue = value;
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
                    it.r() = mFftFalloff[col].mValue(); // falloff value
                    it.g() = magSpectrum[col]; // raw value
                    it.b() = mFftFalloff[col].mValue() * mGain; // falloff w/gain
                    it.a() = 1.0f;
                }
                else
                {
                    if (col < buffer.getNumFrames())
                    {
                        float value = buffer.getChannel(0)[col];
                        it.r() = 0.5f + 0.5f * value * mGain;
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
    mAvgVolume[BAND_MID].mValue = getAverageVolumeByFrequencyRange(mLowPassFilter, mHighPassFilter);
    mAvgVolume[BAND_HIGH].mValue = getAverageVolumeByFrequencyRange(mHighPassFilter, 1.0f);
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
        const float paddingY = 320.0f;
        Rectf preview( windowSize.x - (size+paddingX), windowSize.y - (size+paddingY), windowSize.x-paddingX, windowSize.y - paddingY );
        gl::draw( mAudioFbo.getTexture(), preview );
        
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
    //TODO: audio2 cleanup
    return getAverageVolumeByFrequencyRange( (int)(minRatio * mFftFalloff.size()), (int)(maxRatio * mFftFalloff.size()) );
}

float AudioInputHandler::getAverageVolumeByFrequencyRange(const int minBand /*=0*/, const int maxBand /*=256*/)
{
    float amplitude = 0.0f;
    
    int minIndex = math<int>::max( 0, minBand );
    int maxIndex = math<int>::min( mFftFalloff.size(), maxBand );
    
    for (int32_t i = minIndex; i < maxIndex; i++)
    {
        // TODO: should already be multiplied by gain but it's too low...
        amplitude += mFftFalloff[i].mValue * mGain;
    }
    
    amplitude = amplitude / (float)(maxIndex-minIndex);
    
    return amplitude;
}

