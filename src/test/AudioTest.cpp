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
#include "cinder/Rand.h"
#include "Interface.h"

using namespace ci;
using namespace ci::app;

//AudioTest::AudioTest()
//{
//}
//
//AudioTest::~AudioTest()
//{
//}

void AudioTest::setup()
{
    mUseMotionBlur = false;
    mMotionBlurRenderer.setup( mApp->getWindowSize(), boost::bind( &AudioTest::drawLines, this ) );
}

void AudioTest::setupInterface()
{
    mInterface->addParam(CreateBoolParam( "Motion Blur", &mUseMotionBlur )
                         .defaultValue(mUseMotionBlur));
}

void AudioTest::update(double dt)
{
    Scene::update(dt);
}

void AudioTest::draw()
{
    glPushMatrix();
    gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
    if( mUseMotionBlur )
    {
        mMotionBlurRenderer.draw();
    }
    else
    {
        drawLines();
    }
    glPopMatrix();
}

void AudioTest::drawDebug()
{
    glPushMatrix();
    gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
    
    drawWaveform( mApp->getAudioInput().getPcmBuffer() );
    drawFft( mApp->getAudioInput().getFftDataRef() );
    
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
		float * freqData = audioInput.getFft()->getAmplitude();
		float * timeData = audioInput.getFft()->getData();
		int32_t dataSize = audioInput.getFft()->getBinSize();
        
		// Get dimensions
		float scale = ((float)getWindowWidth() - 20.0f) / (float)dataSize;
		float mWindowHeight = (float)getWindowHeight();
        
		// Use polylines to depict time and frequency domains
		PolyLine<Vec2f> freqLine;
		PolyLine<Vec2f> timeLine;
        
		// Iterate through data
		for (int32_t i = 0; i < dataSize; i++) 
		{
            
			// Do logarithmic plotting for frequency domain
			double mLogSize = log((double)dataSize);
			float x = (float)(log((double)i) / mLogSize) * (double)dataSize;
			float y = math<float>::clamp(freqData[i] * (x / dataSize) * log((double)(dataSize - i)), 0.0f, 2.0f);
            
			// Plot points on lines
			freqLine.push_back(Vec2f(x * scale + 10.0f,           -y * (mWindowHeight - 20.0f) * 0.25f + (mWindowHeight - 10.0f)));
			timeLine.push_back(Vec2f(i * scale + 10.0f, timeData[i] * (mWindowHeight - 20.0f) * 0.25f + (mWindowHeight * 0.25 + 10.0f)));
            
		}
        
		// Draw signals
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

void AudioTest::drawLines()
{
    AudioInput& audioInput = mApp->getAudioInput();
	
    // Get data
    float * freqData = audioInput.getFft()->getAmplitude();
    //float * timeData = audioInput.getFft()->getData();
    int32_t dataSize = audioInput.getFft()->getBinSize();
    
    // Get dimensions
    //float scale = ((float)getWindowWidth() - 20.0f) / (float)dataSize;
    //float mWindowHeight = (float)getWindowHeight();
    
    // Use polylines to depict time and frequency domains
    PolyLine<Vec2f> freqLine;
    PolyLine<Vec2f> timeLine;
    
    
    float screenHeight = getWindowHeight();
    float screenWidth = getWindowWidth();
    //float bottom = getWindowHeight();
    
    glPushMatrix();
    gl::enableAdditiveBlending();
    // Iterate through data
    for (int32_t i = 0; i < dataSize; i++) 
    {
        
        // Do logarithmic plotting for frequency domain
        double logSize = log((double)dataSize);
        float x = (float)(log((double)i) / logSize) * (double)dataSize;
        float y = math<float>::clamp(freqData[i] * (x / dataSize) * log((double)(dataSize - i)), 0.0f, 2.0f);	
    
        const float threshold = 0.025f;
        const float minAlpha = 0.25f;
        if( y > threshold )
        {
            const float width = Rand::randFloat(1.0f+(x/dataSize)*7.0f, (x/dataSize)*10.0f);
            //const float space = Rand::randFloat(1.0f, 3.0f);

            float barX = Rand::randFloat(0.0f,screenWidth);
            glBegin( GL_QUADS );
            // bottom
            glColor4f( 0.0f, 0.1f, 0.75f, y+minAlpha );
            glVertex2f( barX, 0.0f );
            glVertex2f( barX + width, 0.0f );
            // top
            glColor4f( 0.0f, 0.25f, 1.0f, y+minAlpha );
            glVertex2f( barX + width, screenHeight );
            glVertex2f( barX, screenHeight );
            glEnd();
        }
	}
    gl::enableAlphaBlending();
    glPopMatrix();
}
