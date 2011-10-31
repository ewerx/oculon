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
    snprintf(buf, 256, "mw signal: %.2ffps", mindWave.getSignalQuality());
    mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.5f, 0.8f*mindWave.getAttention()));
    snprintf(buf, 256, "mw attn: %.2ffps", mindWave.getAttention());
    mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.5f, 0.8f*mindWave.getAttention()));
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
