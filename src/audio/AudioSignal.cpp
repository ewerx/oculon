/*
 *  AudioSignal.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "OculonApp.h"
#include "AudioInput.h"
#include "AudioSignal.h"
#include "Interface.h"
#include "SignalScope.h"
#include "Barcode.h"
#include "Eclipse.h"
#include "Circles.h"

#include "cinder/Rand.h"
#include <algorithm>

#include <boost/foreach.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;

AudioSignal::AudioSignal()
: Scene("audio")
{
    mSubScenes.push_back( new SignalScope(this) );
    mSubScenes.push_back( new Barcode(this) );
    //mSubScenes.push_back( new Eclipse(this) );
    mSubScenes.push_back( new Circles(this) );
}

AudioSignal::~AudioSignal()
{
    BOOST_FOREACH( SubScene* &ss, mSubScenes )
    {
		delete ss;
	}
}

void AudioSignal::setup()
{
    Scene::setup();
    
    mAudioInputHandler.setup(false);
    
    mUseMotionBlur = false;
    mMotionBlurRenderer.setup( mApp->getViewportSize(), boost::bind( &AudioSignal::drawSubScenes, this ) );
    
    mFilter = 0;
    mFilterFrequency = 0.0f;
    
    BOOST_FOREACH( SubScene* &ss, mSubScenes )
    {
		ss->setup();
	}
}

void AudioSignal::reset()
{
    BOOST_FOREACH( SubScene* &ss, mSubScenes )
    {
		ss->reset();
	}
}

void AudioSignal::setupInterface()
{
    mInterface->addParam(CreateBoolParam( "Motion Blur", &mUseMotionBlur )
                         .oscReceiver(mName,"blur"));
    
    //TODO: audio2 cleanup
    mInterface->addParam(CreateFloatParam( "Filter Freq", &mFilterFrequency )
                         .oscReceiver(mName,"filterfreq"))->registerCallback( this, &AudioSignal::setFilter );
//    mInterface->addEnum(CreateEnumParam("Filter", &mFilter)
//                        .maxValue(Kiss::Filter::NOTCH+1)
//                        .isVertical()
//                        .oscReceiver(mName,"filter"))->registerCallback( this, &AudioSignal::setFilter );
    mInterface->addButton(CreateTriggerParam("Remove Filter", NULL)
                          .oscReceiver(mName,"revemofilter"))->registerCallback( this, &AudioSignal::removeFilter );
    
    BOOST_FOREACH( SubScene* &ss, mSubScenes )
    {
		ss->setupInterface();
	}
    
    mAudioInputHandler.setupInterface(mInterface, mName, 1, 7, 2, 7);
}

//void AudioSignal::setupDebugInterface()
//{
//    Scene::setupDebugInterface();
//    
//    BOOST_FOREACH( SubScene* &ss, mSubScenes )
//    {
//		ss->setupDebugInterface();
//	}
//}

void AudioSignal::update(double dt)
{
    Scene::update(dt);
    
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    BOOST_FOREACH( SubScene* &ss, mSubScenes )
    {
        if( ss->isActive() )
        {
            ss->update(dt);
        }
    }
    
    if( mUseMotionBlur )
    {
        mMotionBlurRenderer.preDraw();
    }
}

void AudioSignal::draw()
{
    gl::pushMatrices();
    //gl::setMatricesWindow( mApp->getViewportWidth(), mApp->getViewportHeight() );
    gl::setMatricesWindowPersp(mApp->getViewportSize());
    if( mUseMotionBlur )
    {
        mMotionBlurRenderer.draw();
    }
    else
    {
        drawSubScenes();
    }
    gl::popMatrices();
}

void AudioSignal::drawSubScenes()
{
    gl::enableAlphaBlending();
    
    BOOST_FOREACH( SubScene* &ss, mSubScenes )
    {
        if( ss->isActive() )
        {
            ss->draw();
        }
    }
}

void AudioSignal::drawDebug()
{
    glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT|GL_CURRENT_BIT);
    gl::pushMatrices();
    gl::setMatricesWindow( mApp->getWindowSize() );
    
    //drawWaveform( mApp->getAudioInput().getPcmBuffer() );
    drawSpectrum( mApp->getAudioInput().getMagSpectrum() );
    
    mAudioInputHandler.drawDebug(mApp->getWindowSize());
    
    gl::popMatrices();
    glPopAttrib();
}

//void AudioSignal::drawWaveform( audio::PcmBuffer32fRef pcmBufferRef )
//{
//    if( ! pcmBufferRef ) 
//    {
//		return;
//	}
//    
//    AudioInput& audioInput = mApp->getAudioInput();
//    
//    gl::pushMatrices();
//    glDisable(GL_LIGHTING);
//    gl::color(ColorA(1.0f,1.0f,1.0f,0.95f));
//	
//    bool useKiss = true;
//    if( useKiss )
//    {
//        // Get data
//        int32_t dataSize = audioInput.getFft()->getBinSize();
//		float * timeData = audioInput.getFft()->getData(); // normalized -1 to +1
//        
//        int32_t	binSize = audioInput.getFft()->getBinSize();
//        float * amplitude = audioInput.getFft()->getAmplitude();
//        //float *	imaginary = audioInput.getFft()->getImaginary();
//        float *	phase = audioInput.getFft()->getPhase();
//        //float *	real = audioInput.getFft()->getReal();
//        
//        const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
//        
//		// Get dimensions
//        const float border = 10.0f;
//		float scaleX = ((float)mApp->getViewportWidth() - border*2.0f) / (float)binSize;
//		float windowHeight = (float)mApp->getViewportHeight();
//        float scaleY = (windowHeight - border*2.0f) * 0.25f;
//        float yOffset = (windowHeight * 0.25f + border);
//        
//        PolyLine<Vec2f> ampLine;
//        PolyLine<Vec2f> phaseLine;
//        PolyLine<Vec2f> imgLine;
//        PolyLine<Vec2f> realLine;
//        
//        for( int32_t i=0; i < binSize; ++i ) 
//        {
//			// Plot points on lines
//			ampLine.push_back(Vec2f(i * scaleX + border, amplitude[i] * (i+1) * scaleY + yOffset + (windowHeight*0.1f)));
//			phaseLine.push_back(Vec2f(i * scaleX + border, phase[i] * (i+1)  * scaleY + yOffset + (windowHeight*0.1f*2.0f)));
//            //imgLine.push_back(Vec2f(i * scaleX + border, imaginary[i] * (i+1)  * scaleY + yOffset + (windowHeight*0.1f*3.0f)));
//            //realLine.push_back(Vec2f(i * scaleX + border, real[i] * (i+1)  * scaleY + yOffset + (windowHeight*0.1f*4.0f)));
//        }
//        
//        scaleX = ((float)mApp->getViewportWidth() - border*2.0f) / (float)dataSize;
//        
//		// Use polylines to depict time and frequency domains
//		PolyLine<Vec2f> freqLine;
//		PolyLine<Vec2f> timeLine;
//        
//		// Iterate through data
//		for (int32_t i = 0; i < dataSize; i++) 
//		{
//            
//			// Do logarithmic plotting for frequency domain
//			//double mLogSize = log((double)dataSize);
//			//float x = (float)(log((double)i) / mLogSize) * (double)dataSize;
//			//float y = math<float>::clamp(freqData[i] * (x / dataSize) * log((double)(dataSize - i)), 0.0f, 2.0f);
//            float x = fftLogData[i].x;
//            float y = fftLogData[i].y;
//            
//			// Plot points on lines
//			freqLine.push_back(Vec2f(x * scaleX + border, -y * scaleY + (windowHeight - border)));
//			timeLine.push_back(Vec2f(i * scaleX + border, timeData[i] * scaleY + (windowHeight * 0.25 + border)));
//            
//		}
//        
//		// Draw signals
//		gl::draw(freqLine);
//		gl::draw(timeLine);
//        gl::color( Color( 0.5f, 0.5f, 1.0f ) );
//        gl::draw(ampLine);
//        //gl::color( Color( 0.5f, 1.0f, 0.5f ) );
//        //gl::draw(phaseLine);
//        gl::color( Color( 1.0f, 0.75f, 0.5f ) );
//        gl::draw(imgLine);
//        gl::color( Color( 1.0f, 0.5f, 0.75f ) );
//        gl::draw(realLine);
//    }
//    else
//    {
//        uint32_t bufferSamples = pcmBufferRef->getSampleCount();
//        audio::Buffer32fRef leftBuffer = pcmBufferRef->getChannelData( audio::CHANNEL_FRONT_LEFT );
//        audio::Buffer32fRef rightBuffer = pcmBufferRef->getChannelData( audio::CHANNEL_FRONT_RIGHT );
//        
//        int displaySize = mApp->getViewportWidth();
//        int endIdx = bufferSamples;
//        
//        //only draw the last 1024 samples or less
//        int32_t startIdx = ( endIdx - 1024 );
//        startIdx = math<int32_t>::clamp( startIdx, 0, endIdx );
//        
//        float scale = displaySize / (float)( endIdx - startIdx );
//        
//        PolyLine<Vec2f>	spectrum_right;
//        PolyLine<Vec2f> spectrum_left;
//        
//        for( uint32_t i = startIdx, c = 0; i < endIdx; i++, c++ ) 
//        {
//            float y = ( ( leftBuffer->mData[i] - 1 ) * - 100 );
//            spectrum_left.push_back( Vec2f( ( c * scale ), y ) );
//            y = ( ( rightBuffer->mData[i] - 1 ) * - 100 );
//            spectrum_right.push_back( Vec2f( ( c * scale ), y ) );
//        }
//        gl::color( Color( 0.5f, 0.5f, 1.0f ) );
//        gl::draw( spectrum_left );
//        gl::color( Color( 1.0f, 0.5f, 0.25f ) );
//        gl::draw( spectrum_right );
//    }
//    gl::popMatrices();
//}

void AudioSignal::drawSpectrum( const vector<float>& magSpectrum )
{
	float height = mApp->getWindowHeight() / 3.0f;
	float bottom = mApp->getWindowHeight() - 80.f;
    float left = 80.0f;
    float width = mApp->getWindowWidth() - left*2.0f;
    
    gl::pushMatrices();
    
    //gl::setMatricesWindow( getWindowSize() );
    mSpectrumPlot.setBounds( Rectf( left, height, width, bottom ) );
    mSpectrumPlot.draw( magSpectrum );
    
    gl::popMatrices();
}

bool AudioSignal::setFilter()
{
    //mApp->getAudioInput().getFft()->setFilter( mFilterFrequency, mFilter );
    return false;
}

bool AudioSignal::removeFilter()
{
//    mFilter = Kiss::Filter::NONE;
//    mFilterFrequency = 0.0f;
//    //mApp->getAudioInput().getFft()->removeFilter();
    return false;
}

bool AudioSignal::handleKeyDown(const ci::app::KeyEvent& keyEvent)
{
    return false;
}
