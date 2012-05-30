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
#include <algorithm>

using namespace ci;
using namespace ci::app;
using namespace std;

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
    mMotionBlurRenderer.setup( mApp->getWindowSize(), boost::bind( &AudioTest::drawVerticalLines, this ) );
    
    // multiwave
    mFalloff = 0.9999f;
    
    mFilter = 0;
    mFilterFrequency = 0.0f;
    
    mIsVerticalOn = false;
    mIsMultiWaveOn = true;
}

void AudioTest::setupInterface()
{
    mInterface->addParam(CreateBoolParam( "Motion Blur", &mUseMotionBlur ));
    mInterface->addParam(CreateBoolParam( "Vertical Lines", &mIsVerticalOn ));
    mInterface->addParam(CreateBoolParam( "MultiWaveform", &mIsMultiWaveOn )
                         .oscReceiver(mName,"filter"));
    mInterface->addParam(CreateFloatParam( "Filter Freq", &mFilterFrequency )
                         .oscReceiver(mName,"filterfreq"))->registerCallback( this, &AudioTest::setFilter );;
    mInterface->addEnum(CreateEnumParam("Filter", &mFilter)
                        .maxValue(Kiss::Filter::NOTCH+1)
                        .isVertical()
                        .oscReceiver(mName,"filter"))->registerCallback( this, &AudioTest::setFilter );
    mInterface->addButton(CreateTriggerParam("Remove Filter", NULL)
                          .oscReceiver(mName,"revemofilter"))->registerCallback( this, &AudioTest::removeFilter );
}

void AudioTest::setupDebugInterface()
{
    mDebugParams.addParam("Falloff", &mFalloff, "step=0.001");
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
        if( mIsVerticalOn )
            drawVerticalLines();
        
        if( mIsMultiWaveOn )
            drawMultiWaveform();
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
        int32_t dataSize = audioInput.getFft()->getBinSize();
		float * timeData = audioInput.getFft()->getData();
        
        int32_t	binSize = audioInput.getFft()->getBinSize();
        float * amplitude = audioInput.getFft()->getAmplitude();
        float *	imaginary = audioInput.getFft()->getImaginary();
        float *	phase = audioInput.getFft()->getPhase();
        float *	real = audioInput.getFft()->getReal();
        
        const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
        
		// Get dimensions
        const float border = 10.0f;
		float scaleX = ((float)getWindowWidth() - border*2.0f) / (float)binSize;
		float windowHeight = (float)getWindowHeight();
        float scaleY = (windowHeight - border*2.0f) * 0.25f;
        float yOffset = (windowHeight * 0.25f + border);
        
        PolyLine<Vec2f> ampLine;
        PolyLine<Vec2f> phaseLine;
        PolyLine<Vec2f> imgLine;
        PolyLine<Vec2f> realLine;
        
        for( int32_t i=0; i < binSize; ++i ) 
        {
			// Plot points on lines
			ampLine.push_back(Vec2f(i * scaleX + border, amplitude[i] * (i+1) * scaleY + yOffset + (windowHeight*0.1f)));
			phaseLine.push_back(Vec2f(i * scaleX + border, phase[i] * (i+1)  * scaleY + yOffset + (windowHeight*0.1f*2.0f)));
            imgLine.push_back(Vec2f(i * scaleX + border, imaginary[i] * (i+1)  * scaleY + yOffset + (windowHeight*0.1f*3.0f)));
            realLine.push_back(Vec2f(i * scaleX + border, real[i] * (i+1)  * scaleY + yOffset + (windowHeight*0.1f*4.0f)));
        }
        
        scaleX = ((float)getWindowWidth() - border*2.0f) / (float)dataSize;
        
		// Use polylines to depict time and frequency domains
		PolyLine<Vec2f> freqLine;
		PolyLine<Vec2f> timeLine;
        
		// Iterate through data
		for (int32_t i = 0; i < dataSize; i++) 
		{
            
			// Do logarithmic plotting for frequency domain
			//double mLogSize = log((double)dataSize);
			//float x = (float)(log((double)i) / mLogSize) * (double)dataSize;
			//float y = math<float>::clamp(freqData[i] * (x / dataSize) * log((double)(dataSize - i)), 0.0f, 2.0f);
            float x = fftLogData[i].x;
            float y = fftLogData[i].y;
            
			// Plot points on lines
			freqLine.push_back(Vec2f(x * scaleX + border, -y * scaleY + (windowHeight - border)));
			timeLine.push_back(Vec2f(i * scaleX + border, timeData[i] * scaleY + (windowHeight * 0.25 + border)));
            
		}
        
		// Draw signals
		gl::draw(freqLine);
		gl::draw(timeLine);
        gl::color( Color( 0.5f, 0.5f, 1.0f ) );
        gl::draw(ampLine);
        //gl::color( Color( 0.5f, 1.0f, 0.5f ) );
        //gl::draw(phaseLine);
        gl::color( Color( 1.0f, 0.75f, 0.5f ) );
        gl::draw(imgLine);
        gl::color( Color( 1.0f, 0.5f, 0.75f ) );
        gl::draw(realLine);
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

void AudioTest::drawVerticalLines()
{
    AudioInput& audioInput = mApp->getAudioInput();
	
    // Get data
    float * freqData = audioInput.getFft()->getAmplitude();
    //float * timeData = audioInput.getFft()->getData();
    int32_t dataSize = audioInput.getFft()->getBinSize();
    const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
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
        //double logSize = log((double)dataSize);
        //float x = (float)(log((double)i) / logSize) * (double)dataSize;
        //float y = math<float>::clamp(freqData[i] * (x / dataSize) * log((double)(dataSize - i)), 0.0f, 2.0f);
        float x = fftLogData[i].x;
        float y = fftLogData[i].y;
    
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

void AudioTest::drawMultiWaveform()
{
    AudioInput& audioInput = mApp->getAudioInput();
    
    // Get data
    float * timeData = audioInput.getFft()->getData();
    float * amplitude = audioInput.getFft()->getAmplitude();
	int32_t	binSize = audioInput.getFft()->getBinSize();
	float *	imaginary = audioInput.getFft()->getImaginary();
	float *	phase = audioInput.getFft()->getPhase();
	float *	real = audioInput.getFft()->getReal();
    int32_t dataSize = audioInput.getFft()->getBinSize();
    const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
    // Get dimensions
    float windowHeight = mApp->getViewportHeight();
    float windowWidth = mApp->getViewportWidth();
    float lineWidth = windowWidth;
    float scaleX = lineWidth / (float)dataSize;
    float xOffset = (windowWidth - lineWidth) / 2.0f; 
    
    const int numLines = 60;
    const float gap = windowHeight / (numLines+1);
    float scaleY = gap * 10.0f;
    float cap = gap * 2.75f;
    float yOffset = windowHeight - gap;
    float falloff = 0.99f;
    
    glColor3f( 1.0f, 1.0f, 1.0f );
    
    //vector<float> sortedAmp(amplitude, amplitude+binSize);
    //sort(sortedAmp.begin(), sortedAmp.end());
    
    gl::enable( GL_LINE_SMOOTH );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	gl::enable( GL_POLYGON_SMOOTH );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	gl::color( ColorAf::white() );
    
    glEnable(GL_POINT_SPRITE);
    glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    glPointSize(3.0f);
    
    
    if( mMultiWaveLines.size() != numLines )
    {
        mMultiWaveLines = vector<tLine>( numLines, tLine(binSize) );
    }
    
    float dampingMin = 0.5f;
    float dampingMax = 0.8f;
    
    Rand indexRand;
    float adjacentY[numLines][binSize];
    const float threshold = 0.0000015f;
    
    for( int lineIndex=0; lineIndex < numLines; ++lineIndex )
    {
        PolyLine<Vec2f> line;
        PolyLine<Vec2f> testLine;
        
        if( mMultiWaveLines[lineIndex].mValue.size() != binSize )
        {
            mMultiWaveLines[lineIndex].mValue = vector<float>( binSize, 0.0f );
        }
        
        int signalType = Rand::randInt(SIGNAL_COUNT);
        
        for( int32_t i=0; i < binSize; ++i ) 
        {
            float scaleFactor = ((binSize/2) - math<float>::abs( (binSize/2) - i )) / (binSize/2);
            
            int bandIndex = Rand::randInt(binSize);
            
            float signal = amplitude[bandIndex];
            switch(signalType)
            {
                case SIGNAL_IMG:
                    signal = imaginary[bandIndex];
                    break;
                    
                case SIGNAL_REAL:
                    signal = real[bandIndex];
                    break;
            }
            
            float y = signal * scaleFactor * scaleY;
            
            float min = 0.0f;
            if( i > 0 && i < binSize-1)
            {
                min = mMultiWaveLines[lineIndex].mValue[i] * falloff;
                
                // dampen height of point to the left
                float damp = mMultiWaveLines[lineIndex].mValue[i-1] * Rand::randFloat(dampingMin,dampingMax);
                
                min = math<float>::max( min, damp );
            }
            y = math<float>::clamp( y, min, cap );
            
            float absY = yOffset - y;
            
            // compare to line below, keep from crossing over
            if( lineIndex > 0 && adjacentY[lineIndex-1][i] < (absY-5) )
            {
                absY = adjacentY[lineIndex-1][i];
                y = mMultiWaveLines[lineIndex-1].mValue[i];
            }
            
            mMultiWaveLines[lineIndex].mValue[i] = y;
            adjacentY[lineIndex][i] = absY;
        }
        
        //indexRand.seed(lineIndex);
        for( int32_t i=0; i < binSize; ++i ) 
        {
            // dampen height of point to the right
            float damp = mMultiWaveLines[lineIndex].mValue[i+1] * Rand::randFloat(dampingMin,dampingMax);
            
            mMultiWaveLines[lineIndex].mValue[i] = math<float>::max( mMultiWaveLines[lineIndex].mValue[i], damp );
            
            float absY = yOffset - mMultiWaveLines[lineIndex].mValue[i];
			line.push_back(Vec2f(i * scaleX + xOffset, absY));
        }
        
        //gl::color( Color(1.0f,1.0f,1.0f) );
        gl::draw(line);
        //gl::color( Color(1.0f,0.0f,0.0f) );
        //gl::draw(testLine);
        yOffset -= gap;
    }    
}

bool AudioTest::setFilter()
{
    mApp->getAudioInput().getFft()->setFilter( mFilterFrequency, mFilter );
    return false;
}

bool AudioTest::removeFilter()
{
    mFilter = Kiss::Filter::NONE;
    mFilterFrequency = 0.0f;
    mApp->getAudioInput().getFft()->removeFilter();
    return false;
}
