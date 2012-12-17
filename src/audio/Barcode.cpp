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
    
    mFalloff = 1.0f;
    mFftFalloff = true;
    mFalloffMode = FALLOFF_OUTEXPO;
    
    mNumBars = 1;
    mBarGap = 0;
    mVertical = true;
    
    mFftMode = FFT_ALPHA;
    
    for (int i=0; i < MAX_BARS; ++i)
    {
        for (int j=0; j < MAX_LINES; ++j)
        {
            mLines[i][j].mAlpha = 0.0f;
            mLines[i][j].mWidth = 0.0f;
        }
    }
    
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
                        .maxValue(MAX_BARS)
                        .oscReceiver(mName,"numbars"));
    
    interface->addParam(CreateFloatParam("Falloff", &mFalloff)
                        .minValue(0.0f)
                        .maxValue(20.0f));
    interface->addParam(CreateBoolParam("FFT Falloff", &mFftFalloff));
    interface->addEnum(CreateEnumParam("Falloff Mode", (int*)&mFalloffMode)
                       .maxValue(FALLOFF_COUNT)
                       .oscReceiver(mName,"falloffmode")
                       .isVertical());
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
    int32_t dataSize = math<int>::max( audioInput.getFft()->getBinSize(), MAX_LINES );
    const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
    // Iterate through data
    for (int32_t i = 0; i < dataSize; i++)
    {
        float x = fftLogData[i].x;
        float y = fftLogData[i].y;
        
        float falloff = mFftFalloff ? (mFalloff * (1.0f - x / dataSize)) : mFalloff;
        
        if( y > mThreshold )
        {
            // width
            float width;
            if( mWidthByFft )
            {
                width = mBaseWidth+(1.0f-x/dataSize)*mMaxWidth;
            }
            else
            {
                width = mBaseWidth;
            }
        
            if (width > mLines[index][i].mWidth)
            {
                mLines[index][i].mWidth = width;
                timeline().apply( &mLines[index][i].mWidth, 0.0f, falloff, getFalloffFunction() );
            }
            width = mLines[index][i].mWidth;
            
            // position
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
            
            // alpha
            ColorA color( mColor.r, mColor.g, mColor.b, mColor.a );
            float alpha = color.a;
            if( mAlphaByFft )
            {
                alpha = mColor.a + y;
                if( alpha > mLines[index][i].mAlpha )
                {
                    mLines[index][i].mAlpha = alpha;
                    timeline().apply( &mLines[index][i].mAlpha, 0.0f, falloff, getFalloffFunction() );
                }
                color.a = mLines[index][i].mAlpha;
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

Barcode::tEaseFn Barcode::getFalloffFunction()
{
    switch( mFalloffMode )
    {
        case FALLOFF_LINEAR: return EaseNone();
        case FALLOFF_OUTQUAD: return EaseOutQuad();
        case FALLOFF_OUTEXPO: return EaseOutExpo();
        case FALLOFF_OUTBACK: return EaseOutBack();
        case FALLOFF_OUTBOUNCE: return EaseOutBounce();
        case FALLOFF_OUTINEXPO: return EaseOutInExpo();
        case FALLOFF_OUTINBACK: return EaseOutInBack();
            
        default: return EaseNone();
    }
}
