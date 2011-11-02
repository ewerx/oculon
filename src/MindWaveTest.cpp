/*
 *  MindWaveTest.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "OculonApp.h"//TODO: fix this dependency
#include "MindWave.h"
#include "MindWaveTest.h"
#include <iostream>

//MindWaveTest::MindWaveTest()
//{
//}
//
//MindWaveTest::~MindWaveTest()
//{
//}

void MindWaveTest::setup()
{
}

void MindWaveTest::update(double /*dt*/)
{
    MindWave& mindWave = mApp->getMindWave();
    char buf[256];
    snprintf(buf, 256, "mw signal: %.1f", mindWave.getSignalQuality());
    mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.5f, 0.8f*mindWave.getSignalQuality()));
    
    snprintf(buf, 256, "mw attn: %.1f", mindWave.getAttention());
    mApp->getInfoPanel().addLine(buf, Color(0.8f*mindWave.getAttention(), 0.5f, 0.8f));
    
    snprintf(buf, 256, "mw meditation: %.1f", mindWave.getMeditation());
    mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.5f, 0.8f*mindWave.getMeditation()));
    
    snprintf(buf, 256, "mw alpha1: %.1f", mindWave.getAlpha1());
    mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.5f, 0.8f*mindWave.getAlpha1()));
    snprintf(buf, 256, "mw alpha2: %.1f", mindWave.getAlpha2());
    mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.5f, 0.8f*mindWave.getAlpha2()));
    snprintf(buf, 256, "mw beta1: %.1f", mindWave.getBeta1());
    mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.5f, 0.8f*mindWave.getBeta1()));
    snprintf(buf, 256, "mw beta1: %.1f", mindWave.getBeta2());
    mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.5f, 0.8f*mindWave.getBeta2()));
    snprintf(buf, 256, "mw gamma1: %.1f", mindWave.getGamma1());
    mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.5f, 0.8f*mindWave.getGamma1()));
    snprintf(buf, 256, "mw gamma2: %.1f", mindWave.getGamma2());
    mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.5f, 0.8f*mindWave.getGamma2()));
    snprintf(buf, 256, "mw delta: %.1f", mindWave.getDelta());
    mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.5f, 0.8f*mindWave.getDelta()));
    snprintf(buf, 256, "mw theta: %.1f", mindWave.getTheta());
    mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.5f, 0.8f*mindWave.getTheta()));
    snprintf(buf, 256, "mw raw: %.1f", mindWave.getRaw());
    mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.5f, 0.8f*mindWave.getRaw()));
    
    mRawValues.push_back(mindWave.getRaw());
    if(mRawValues.size() == 1024)
    {
        mRawValues.erase(mRawValues.begin());
    }
}

void MindWaveTest::draw()
{
    MindWave& mindWave = mApp->getMindWave();
    
    glPushMatrix();
    {
        gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
        // clear out the window with black
        //gl::clear( Color( 0, 0, 0 ) ); 
        
        glTranslatef( 0.0f, 200.0f, 0.0f );
        drawGraphs();
    }
    glPopMatrix();
}

void MindWaveTest::drawGraphs()
{
    MindWave& mindWave = mApp->getMindWave();
    
    int displaySize = getWindowWidth();
    
    //only draw the last 1024 samples or less
    const int numSamples = 1024;
    
    float scale = displaySize / (float)( numSamples );
    
    PolyLine<Vec2f>	spectrum;
    int c = 0;
    
    for(vector<float>::iterator it = mRawValues.begin(); it != mRawValues.end(); ++it)
    {
        float y = (*it) / 100.f;
        spectrum.push_back( Vec2f( ( c * scale ), y ) );
        ++c;
    }
    gl::color( Color( 1.0f, 0.5f, 0.5f ) );
    gl::draw( spectrum );
    
    /*
    uint16_t bandCount = audioInput.getFftBandCount();
	float ht = 1000.0f;
	float bottom = 150.0f;
	
	if( ! fftDataRef ) 
    {
		return;
	}
	
	float * fftBuffer = fftDataRef.get();
	
	for( int i = 0; i < ( bandCount ); i++ ) 
    {
		float barY = fftBuffer[i] / bandCount * ht;
		glBegin( GL_QUADS );
        glColor3f( 255.0f, 255.0f, 0.0f );
        glVertex2f( i * 3, bottom );
        glVertex2f( i * 3 + 1, bottom );
        glColor3f( 0.0f, 255.0f, 0.0f );
        glVertex2f( i * 3 + 1, bottom - barY );
        glVertex2f( i * 3, bottom - barY );
		glEnd();
	}
    */
}
