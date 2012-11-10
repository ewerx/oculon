/*
 *  Barcode.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */


#include "Barcode.h"
#include "Utils.h"
#include "Scene.h"
#include "OculonApp.h"
#include "Interface.h"
#include "Constants.h"

#include "cinder/gl/gl.h"
#include "cinder/CinderMath.h"
#include "cinder/Rand.h"
#include "cinder/Easing.h"

using namespace ci;
using namespace std;


//
// Barcode
// 

Barcode::Barcode(Scene* scene)
: SubScene(scene,"lines")
{
}

Barcode::~Barcode()
{
}

void Barcode::setup()
{
    mColor = ColorAf(1.0f,1.0f,1.0f,0.25f);
    
    mBaseWidth = 1.0f;
    mMaxWidth = 15.0f;
    mThreshold = 0.02f;
    mWidthByFft = true;
    mAlphaByFft = true;
    mPositionByFft = false;
    
    mNumBars = 1;
    mBarGap = 0;
    mVertical = true;
    
    mFftMode = FFT_ALPHA;
    
    reset();
}

void Barcode::setupInterface()
{
    SubScene::setupInterface();
    
    Interface* interface = mParentScene->getInterface();
    
    interface->addParam(CreateBoolParam("Width By Audio", &mWidthByFft)
                        .oscReceiver(mName,"widthaudio"));
    interface->addParam(CreateBoolParam("Alpha By Audio", &mAlphaByFft)
                        .oscReceiver(mName,"alphaaudio"));
    interface->addParam(CreateBoolParam("Pos By Audio", &mPositionByFft)
                        .oscReceiver(mName,"posaudio"));
    interface->addParam(CreateFloatParam("Base Width", &mBaseWidth)
                        .minValue(1.0f)
                        .maxValue(10.0f)
                        .oscReceiver(mName,"basewidth"));
    interface->addParam(CreateFloatParam("Max Width", &mMaxWidth)
                        .minValue(1.0f)
                        .maxValue(50.0f)
                        .oscReceiver(mName,"maxwidth"));
    interface->addParam(CreateFloatParam("Threshold", &mThreshold)
                        .maxValue(0.1f)
                        .oscReceiver(mName,"threshold"));
    interface->addParam(CreateColorParam("Color", &mColor, kMinColor, kMaxColor)
                        .oscReceiver(mName,"color"));
    
    interface->addParam(CreateBoolParam("Vetical", &mVertical)
                        .oscReceiver(mName,"vertical"));
    interface->addParam(CreateIntParam("Num Bars", &mNumBars)
                        .maxValue(64)
                        .oscReceiver(mName,"numbars"));
}

void Barcode::setupDebugInterface()
{
}

void Barcode::reset()
{
}

void Barcode::resize()
{
}

void Barcode::update(double dt)
{
}

void Barcode::draw()
{
    gl::pushMatrices();
    gl::enableAdditiveBlending();
    
    for( int i=0; i < mNumBars; ++i )
    {
        drawBar(i);
    }
    
    gl::enableAlphaBlending();
    gl::popMatrices();

}

void Barcode::drawBar(const int index)
{
    float screenWidth = mParentScene->getApp()->getViewportWidth();
    float screenHeight = mParentScene->getApp()->getViewportHeight();
    
    // vertical barcodes are split into horizontal segments
    // horizontal barcodes are split into vertical segments
    float barSize = mVertical ? (screenHeight / mNumBars) : (screenWidth / mNumBars);
    float barRange = mVertical ? screenWidth : screenHeight;
    float barOffset = barSize * index;

    AudioInput& audioInput = mParentScene->getApp()->getAudioInput();
	
    // Get data
    //float * freqData = audioInput.getFft()->getAmplitude();
    //float * timeData = audioInput.getFft()->getData();
    int32_t dataSize = audioInput.getFft()->getBinSize();
    const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
    // Iterate through data
    for (int32_t i = 0; i < dataSize; i++)
    {
        float x = fftLogData[i].x;
        float y = fftLogData[i].y;
        
        if( y > mThreshold )
        {
            float width;
            if( mWidthByFft )
            {
                width = mBaseWidth+(1.0f-x/dataSize)*mMaxWidth;
            }
            else
            {
                width = mBaseWidth;
            }
            
            float lineX;
            if( mPositionByFft )
            {
                if( Rand::randFloat() < 0.5f )
                {
                    lineX = (barRange/2.0f) + (x/dataSize)*(barRange/2.0f);
                }
                else
                {
                    lineX = (barRange/2.0f) - (x/dataSize)*(barRange/2.0f);
                }
            }
            else
            {
                lineX = Rand::randFloat(0.0f,barRange);
            }
            
            ColorA color( mColor.r, mColor.g, mColor.b, mColor.a );
            if( mAlphaByFft )
            {
                color.a = mColor.a + y;
            }
            
            gl::color( color );
            
            glBegin( GL_QUADS );
            if( mVertical )
            {
                // bottom
                glVertex2f( lineX,          barOffset );
                glVertex2f( lineX + width,  barOffset );
                // top
                glVertex2f( lineX + width,  barOffset + barSize );
                glVertex2f( lineX,          barOffset + barSize );
            }
            else
            {
                // left
                glVertex2f( barOffset, lineX );
                glVertex2f( barOffset, lineX + width );
                // right
                glVertex2f( barOffset + barSize, lineX + width );
                glVertex2f( barOffset + barSize, lineX );
            }
            glEnd();
        }
	}
}
