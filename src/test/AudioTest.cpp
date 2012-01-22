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

void AudioTest::update(double /*dt*/)
{
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
		float * freqData = audioInput.getFrequencyData();//audioInput.getFft().getAmplitude();
		float * timeData = audioInput.getTimeData();//audioInput.getFft().getData();
		int32_t dataSize = audioInput.getFft()->getBinSize();
        
        if( !freqData || !timeData )
        {
            return;
        }
        
		// Get dimensions
		float scale = ((float)getWindowWidth() - 20.0f) / (float)dataSize;
		float windowHeight = (float)getWindowHeight();
        
		// Use polylines to depict time and frequency domains
		PolyLine<Vec2f> freqLine;
		PolyLine<Vec2f> timeLine;
        
        // Do logarithmic plotting for frequency domain
        double logSize = log((double)dataSize);
        
        float ht = getWindowHeight() * 0.25f;
        float bottom = getWindowHeight() - 250.f;
        const float width = 2.0f;
        const float space = width + 0.0f;
        
        glPushMatrix();
		// Iterate through data
		for (int32_t i = 0; i < dataSize; i++) 
		{
			float x = (float)(log((double)i) / logSize) * (double)dataSize;
			float y = math<float>::clamp(freqData[i] * (x / dataSize) * log((double)(dataSize - i)), 0.0f, 2.0f);
            
            float barY = y * ht;
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
            
			// Plot points on lines
			freqLine.push_back(Vec2f(x * scale + 10.0f,           -y * (windowHeight - 20.0f) * 0.25f + (windowHeight - 10.0f)));
			timeLine.push_back(Vec2f(i * scale + 10.0f, timeData[i] * (windowHeight - 20.0f) * 0.25f + (windowHeight * 0.25 + 10.0f)));
            
            //console() << i << ": " << mTimeData[i] << std::endl;
            
		}
        //console() << "--------------" << std::endl;
        glPopMatrix();
        
		// Draw signals
        glColor4f(1.0f,1.0f,1.0f,0.95f);
		gl::draw(freqLine);
		gl::draw(timeLine);
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
