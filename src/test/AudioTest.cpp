/*
 *  AudioTest.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "OculonApp.h"//TODO: fix this dependency
#include "AudioInput.h"
#include "AudioTest.h"
#include "KissFFT.h"

//AudioTest::AudioTest()
//{
//}
//
//AudioTest::~AudioTest()
//{
//}

void AudioTest::setup()
{
}

void AudioTest::update(double dt)
{
    Scene::update(dt);
}

void AudioTest::draw()
{
    AudioInput& audioInput = mApp->getAudioInput();
    
    glPushMatrix();
    static bool drawGraphs = true;
    if( drawGraphs )
    {
        gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
        // clear out the window with black
        //gl::clear( Color( 0, 0, 0 ) ); 
        
        drawWaveform( audioInput.getPcmBuffer() );
        drawFft( audioInput.getFftDataRef() );
    }
    glPopMatrix();
}


void AudioTest::drawWaveform( audio::PcmBuffer32fRef pcmBufferRef )
{
    if( ! pcmBufferRef ) 
    {
		return;
	}
    
    AudioInput& audioInput = mApp->getAudioInput();
    
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glColor4f(1.0f,1.0f,1.0f,0.95f);
	
    bool useKiss = true;
    if( useKiss )
    {
        // Get data
		float * mFreqData = audioInput.getFft()->getAmplitude();
		float * mTimeData = audioInput.getFft()->getData();
		int32_t mDataSize = audioInput.getFft()->getBinSize();
        
		// Get dimensions
		float mScale = ((float)getWindowWidth() - 20.0f) / (float)mDataSize;
		float mWindowHeight = (float)getWindowHeight();
        
		// Use polylines to depict time and frequency domains
		PolyLine<Vec2f> mFreqLine;
		PolyLine<Vec2f> mTimeLine;
        
		// Iterate through data
		for (int32_t i = 0; i < mDataSize; i++) 
		{
            
			// Do logarithmic plotting for frequency domain
			double mLogSize = log((double)mDataSize);
			float x = (float)(log((double)i) / mLogSize) * (double)mDataSize;
			float y = math<float>::clamp(mFreqData[i] * (x / mDataSize) * log((double)(mDataSize - i)), 0.0f, 2.0f);
            
			// Plot points on lines
			mFreqLine.push_back(Vec2f(x * mScale + 10.0f,           -y * (mWindowHeight - 20.0f) * 0.25f + (mWindowHeight - 10.0f)));
			mTimeLine.push_back(Vec2f(i * mScale + 10.0f, mTimeData[i] * (mWindowHeight - 20.0f) * 0.25f + (mWindowHeight * 0.25 + 10.0f)));
            
		}
        
		// Draw signals
		gl::draw(mFreqLine);
		gl::draw(mTimeLine);
    }
    else
    {
	uint32_t bufferSamples = pcmBufferRef->getSampleCount();
	audio::Buffer32fRef leftBuffer = pcmBufferRef->getChannelData( audio::CHANNEL_FRONT_LEFT );
	audio::Buffer32fRef rightBuffer = pcmBufferRef->getChannelData( audio::CHANNEL_FRONT_RIGHT );
    
	int displaySize = getWindowWidth();
	int endIdx = bufferSamples;
	
	//only draw the last 1024 samples or less
	int32_t startIdx = ( endIdx - 1024 );
	startIdx = math<int32_t>::clamp( startIdx, 0, endIdx );
	
	float scale = displaySize / (float)( endIdx - startIdx );
	
	PolyLine<Vec2f>	spectrum_right;
    PolyLine<Vec2f> spectrum_left;
	
	for( uint32_t i = startIdx, c = 0; i < endIdx; i++, c++ ) 
    {
        float y = ( ( leftBuffer->mData[i] - 1 ) * - 100 );
		spectrum_left.push_back( Vec2f( ( c * scale ), y ) );
        y = ( ( rightBuffer->mData[i] - 1 ) * - 100 );
		spectrum_right.push_back( Vec2f( ( c * scale ), y ) );
	}
    gl::color( Color( 0.5f, 0.5f, 1.0f ) );
    gl::draw( spectrum_left );
    gl::color( Color( 1.0f, 0.5f, 0.25f ) );
	gl::draw( spectrum_right );
    }
    glPopMatrix();
}

void AudioTest::drawFft( std::shared_ptr<float> fftDataRef )
{
    AudioInput& audioInput = mApp->getAudioInput();
    uint16_t bandCount = audioInput.getFftBandCount();
	float ht = getWindowHeight() * 0.70f;
	float bottom = getWindowHeight() - 50.f;
    const float width = 2.0f;
    const float space = width + 0.0f;
	
	if( ! fftDataRef ) 
    {
		return;
	}
	
    glPushMatrix();
	float * fftBuffer = fftDataRef.get();
	
	for( int i = 0; i < ( bandCount ); i++ ) 
    {
		float barY = fftBuffer[i] / bandCount * ht;
		glBegin( GL_QUADS );
        // bottom
        glColor3f( 0.25f, 0.0f, 0.0f );
        glVertex2f( i * space, bottom );
        glVertex2f( i * space + width, bottom );
        // top
        glColor3f( 1.0f, 0.25f, 0.0f );
        glVertex2f( i * space + width, bottom - barY );
        glVertex2f( i * space, bottom - barY );
		glEnd();
	}
    glPopMatrix();
}
