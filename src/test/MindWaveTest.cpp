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
    Scene::setup();
    
    mMaxValues = 1024;
    
    /*
    // Set line dimensions
	mAmplitude = 0.0025f;
	mOffset = 30.0f;
	mNumPoints = 24;
	mRadius = 300.0f;
	mRotation = 0.0f;
	mSpeed = 0.333f;
	mTrails = 0.0333f;
    
	// Initialize waves
	mAlpha1.moveTo( 0.0f, 0.0f );
    mAlpha2.moveTo( 0.0f, 0.0f );
	mBeta1.moveTo( 0.0f, 0.0f );
    mBeta2.moveTo( 0.0f, 0.0f );
    mGamma1.moveTo( 0.0f, 0.0f );
    mGamma2.moveTo( 0.0f, 0.0f );
	mDelta.moveTo( 0.0f, 0.0f );
	mTheta.moveTo( 0.0f, 0.0f );
	for ( uint32_t i = 0; i < mNumPoints; i++ ) {
		float x = ( (float)i / (float)mNumPoints ) * mRadius;
		mAlpha1.lineTo( x, 0.0f );
        mAlpha2.lineTo( x, 0.0f );
        mBeta1.lineTo( x, 0.0f );
        mBeta2.lineTo( x, 0.0f );
        mGamma1.lineTo( x, 0.0f );
        mGamma2.lineTo( x, 0.0f );
		mDelta.lineTo( x, 0.0f );
		mTheta.lineTo( x, 0.0f );
	}
	mNumPoints = mAlpha1.getNumPoints();
    
	// Define colors
    mColorBackground = Colorf( 0.207f, 0.176f, 0.223f );
	mColorAlpha = ColorAf( 0.207f, 1.0f, 1.0f, 1.0f );
	mColorBeta  = ColorAf( 1.0f, 0.836f, 0.895f, 1.0f );
    mColorGamma = ColorAf( 1.0f, 0.836f, 0.495f, 1.0f );
	mColorDelta = ColorAf( 0.531f, 0.0f, 0.223f, 1.0f );
	mColorTheta = ColorAf( 0.531f, 0.375f, 0.828f, 1.0f );
    */
}

void MindWaveTest::update(double dt)
{
    MindWave& mindWave = mApp->getMindWave();
    
    char buf[256];
    snprintf(buf, 256, "mw signal: %.1f", mindWave.getSignalQuality());
    mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.5f, 0.8f*mindWave.getSignalQuality()));
    snprintf(buf, 256, "mw battery: %.1f", mindWave.getBattery());
    mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.5f, 0.8f*mindWave.getBattery()));
    snprintf(buf, 256, "mw blink: %.1f", mindWave.getBlink());
    mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.5f, 0.8f*mindWave.getBlink()));
    
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
    
    if( mindWave.hasData() )
    {
        mRawValues.push_back(mindWave.getRaw());
        if(mRawValues.size() == mMaxValues)
        {
            mRawValues.erase(mRawValues.begin());
        }
        
        mMeditation.push_back(mindWave.getMeditation());
        if(mMeditation.size() == mMaxValues)
        {
            mMeditation.erase(mMeditation.begin());
        }
        
        mAttention.push_back(mindWave.getAttention());
        if(mAttention.size() == mMaxValues)
        {
            mAttention.erase(mAttention.begin());
        }
    }
    /*
    if( mindWave.hasData() )
    {
        // Shift points over by one
        for ( uint32_t i = mNumPoints - 1; i > 0; i-- ) {
            mAlpha1.setPoint( i, Vec2f( mAlpha1.getPoint( i ).x, mAlpha1.getPoint( i - 1 ).y ) );
            mAlpha2.setPoint( i, Vec2f( mAlpha2.getPoint( i ).x, mAlpha2.getPoint( i - 1 ).y ) );
            mBeta1.setPoint(  i, Vec2f( mBeta1.getPoint(  i ).x, mBeta1.getPoint(  i - 1 ).y ) );
            mBeta2.setPoint(  i, Vec2f( mBeta2.getPoint(  i ).x, mBeta2.getPoint(  i - 1 ).y ) );
            mGamma1.setPoint(  i, Vec2f( mGamma1.getPoint(  i ).x, mGamma1.getPoint(  i - 1 ).y ) );
            mGamma2.setPoint(  i, Vec2f( mGamma2.getPoint(  i ).x, mGamma2.getPoint(  i - 1 ).y ) );
            mDelta.setPoint( i, Vec2f( mDelta.getPoint( i ).x, mDelta.getPoint( i - 1 ).y ) );
            mTheta.setPoint( i, Vec2f( mTheta.getPoint( i ).x, mTheta.getPoint( i - 1 ).y ) );
        }
        
        // Set value of brainwave channel in first position of each line
        mAlpha1.setPoint(0, Vec2f( mAlpha1.getPoint( 0 ).x, -mAmplitude * 0.5f + mindWave.getAlpha1() * mAmplitude ) );
        mAlpha2.setPoint(0, Vec2f( mAlpha2.getPoint( 0 ).x, -mAmplitude * 0.5f + mindWave.getAlpha2() * mAmplitude ) );
        mBeta1.setPoint( 0, Vec2f( mBeta1.getPoint(  0 ).x, -mAmplitude * 0.5f + mindWave.getBeta1() * mAmplitude ) );
        mBeta2.setPoint( 0, Vec2f( mBeta2.getPoint(  0 ).x, -mAmplitude * 0.5f + mindWave.getBeta2() * mAmplitude ) );
        mGamma1.setPoint( 0, Vec2f( mGamma1.getPoint(  0 ).x, -mAmplitude * 0.5f + mindWave.getGamma1() * mAmplitude ) );
        mGamma2.setPoint( 0, Vec2f( mGamma2.getPoint(  0 ).x, -mAmplitude * 0.5f + mindWave.getGamma2() * mAmplitude ) );
        mDelta.setPoint(0, Vec2f( mDelta.getPoint( 0 ).x, -mAmplitude * 0.5f + mindWave.getDelta() * mAmplitude ) );
        mTheta.setPoint(0, Vec2f( mTheta.getPoint( 0 ).x, -mAmplitude * 0.5f + mindWave.getTheta() * mAmplitude ) );
    }
    
    // Update overall rotation
	mRotation += mSpeed;
	if ( mRotation > 360.0f ) {
		mRotation -= 360.0f;
	}
    */
    Scene::update(dt);
}

void MindWaveTest::draw()
{
    // Set up OpenGL
	gl::enable( GL_BLEND );
	gl::enable( GL_LINE_SMOOTH );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	gl::enableAlphaBlending( true );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    
    gl::pushMatrices();
    {
        gl::setMatricesWindow( mApp->getViewportWidth(), mApp->getViewportHeight() );
        
        drawGraphs();
    }
    gl::popMatrices();
}

void MindWaveTest::drawGraphs()
{
    /*
    // Draw a box over the entire display instead of clearing
	// the screen. This will leave trails.
	gl::color( ColorAf( mColorBackground, mTrails ) );
	gl::drawSolidRect( Rectf( getViewportBounds() ) );
    
	// Rotate screen around center
	Vec2f center = getViewportCenter();
	gl::pushMatrices();
	gl::translate( center );
	gl::rotate( mRotation );
	gl::translate( center * -1.0f );
	gl::translate( center );
    
	// Draw alpha waves
	gl::color( mColorAlpha );
	gl::pushMatrices();
	gl::translate( mOffset, 0.0f );
	gl::draw( mAlpha1 );
	gl::popMatrices();
    
//    gl::pushMatrices();
//    gl::rotate( 45.0f );
//	gl::translate( mOffset, 0.0f );
//	gl::draw( mAlpha2 );
//	gl::popMatrices();
    
	// Beta
	gl::color( mColorBeta );
	gl::pushMatrices();
	gl::rotate( 90.0f );
	gl::translate( mOffset, 0.0f );
	gl::draw( mBeta1 );
	gl::popMatrices();
    
	// Delta
	gl::color( mColorDelta );
	gl::pushMatrices();
	gl::rotate( 180.0f );
	gl::translate( mOffset, 0.0f );
	gl::draw( mDelta );
	gl::popMatrices();
    
	// Theta
	gl::color( mColorTheta );
	gl::pushMatrices();
	gl::rotate( 270.0f );
	gl::translate( mOffset, 0.0f );
	gl::draw( mTheta );
	gl::popMatrices();
    
    // Stop drawing
	gl::popMatrices();
    */
    
    MindWave& mindWave = mApp->getMindWave();
    if( mindWave.hasData() && mindWave.getBlink() > 0.0f )
    {
        gl::color(ColorA(1.0f,1.0f,1.0f,0.8f));
        gl::drawSolidRect( Rectf( mApp->getViewportBounds() ) );
        console() << "[mindwave] blink!\n";
    }

    int displaySize = mApp->getViewportWidth();
    
    //only draw the last 1024 samples or less
    const int numSamples = 1024;
    
    float scale = displaySize / (float)( numSamples );
    
    PolyLine<Vec2f> raw;
    PolyLine<Vec2f>	attention;
    PolyLine<Vec2f>	meditation;
    int c = 0;
    
    for(vector<float>::iterator it = mRawValues.begin(); it != mRawValues.end(); ++it)
    {
        float y = (*it) / 50.f;
        raw.push_back( Vec2f( ( c * scale ), -y ) );
        ++c;
    }
    c=0;
    for(vector<float>::iterator it = mMeditation.begin(); it != mMeditation.end(); ++it)
    {
        float y = (*it);
        meditation.push_back( Vec2f( ( c * scale ), -y ) );
        ++c;
    }
    c=0;
    for(vector<float>::iterator it = mAttention.begin(); it != mAttention.end(); ++it)
    {
        float y = (*it);
        attention.push_back( Vec2f( ( c * scale ), -y ) );
        ++c;
    }
    
    gl::pushMatrices();
    gl::translate( 0.0f, 200.0f );
    gl::color( Color( 1.0f, 0.75f, 0.5f ) );
    gl::draw( raw );
    
    gl::translate( 0.0f, 200.0f );
    gl::color( Color( 1.0f, 0.2f, 0.2f ) );
    gl::draw( attention );
    
    gl::translate( 0.0f, 200.0f );
    gl::color( Color( 0.5f, 1.0f, 0.75f ) );
    gl::draw( meditation );
    gl::popMatrices();
    
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
