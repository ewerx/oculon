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
using namespace ci::app;
using namespace std;


//
// Barcode
// 

Barcode::Barcode(Scene* scene)
: SubScene(scene,"barcode")
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
    
    mPositionMode = POSITION_SHIFT_RANDOM;
    mWidthMode = WIDTH_AUDIO_FREQ;
    
    mNumBars = 1;
    mBarGap = 0;
    mVertical = true;
    
    for (int i=0; i < MAX_BARS; ++i)
    {
        for (int j=0; j < MAX_LINES; ++j)
        {
            mLines[i][j].mAlpha = 0.0f;
            mLines[i][j].mWidth = 0.0f;
        }
    }
    
    mIsActive = true;
    
    reset();
}

void Barcode::setupInterface()
{
    SubScene::setupInterface();
    
    Interface* interface = mParentScene->getInterface();
    
    interface->addParam(CreateBoolParam("Width By Audio", &mWidthByFft)
                        .oscReceiver(getName(),"widthaudio"));
    interface->addParam(CreateBoolParam("Alpha By Audio", &mAlphaByFft)
                        .oscReceiver(getName(),"alphaaudio"));
    interface->addParam(CreateFloatParam("Base Width", &mBaseWidth)
                        .minValue(1.0f)
                        .maxValue(10.0f)
                        .oscReceiver(getName(),"basewidth")
                        .midiInput(0, 2, 6));
    interface->addParam(CreateFloatParam("Max Width", &mMaxWidth)
                        .minValue(1.0f)
                        .maxValue(50.0f)
                        .oscReceiver(getName(),"maxwidth")
                        .midiInput(0, 1, 6));
    interface->addParam(CreateFloatParam("Threshold", &mThreshold)
                        .maxValue(0.1f)
                        .oscReceiver(getName(),"threshold"));
    interface->addParam(CreateColorParam("Color", &mColor, kMinColor, kMaxColor)
                        .oscReceiver(getName(),"color"));
    
    interface->addParam(CreateBoolParam("Vetical", &mVertical)
                        .oscReceiver(getName(),"vertical")
                        .sendFeedback()
                        .midiInput(0, 2, 5));
    interface->addParam(CreateIntParam("Num Bars", &mNumBars)
                        .maxValue(MAX_BARS)
                        .oscReceiver(getName(),"numbars")
                        .midiInput(0, 1, 5));
    
    vector<string> posModeNames;
#define BARCODE_POS_MODE_ENTRY( nam, enm ) \
posModeNames.push_back(nam);
    BARCODE_POS_MODE_TUPLE
#undef  BARCODE_POS_MODE_ENTRY
    interface->addEnum(CreateEnumParam("Position Mode", (int*)&mPositionMode)
                       .maxValue(POSITION_COUNT)
                       .oscReceiver(getName(),"posmode")
                       .isVertical(), posModeNames);
    
    vector<string> widthModeNames;
#define BARCODE_WIDTH_MODE_ENTRY( nam, enm ) \
widthModeNames.push_back(nam);
    BARCODE_WIDTH_MODE_TUPLE
#undef  BARCODE_WIDTH_MODE_ENTRY
    interface->addEnum(CreateEnumParam("Width Mode", (int*)&mWidthMode)
                       .maxValue(WIDTH_MODE_COUNT)
                       .oscReceiver(getName(),"widthmode")
                       .isVertical(), widthModeNames);
}

//void Barcode::setupDebugInterface()
//{
//}

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
	
    AudioInputHandler::FftValues& fftValues = mParentScene->getAudioInputHandler().getFftValues();
    int32_t dataSize = fftValues.size();
    
    //TODO: isn't the falloff done inside the AudioInputHandler???
    AudioInputHandler::tEaseFn easeFn = mParentScene->getAudioInputHandler().getFalloffFunction();
    float falloff = mParentScene->getAudioInputHandler().mFalloffTime;
    
    // Iterate through data
    for (int32_t i = 0; i < dataSize; i++)
    {
        float x = fftValues[i].mBandIndex;
        float y = fftValues[i].mValue;
        
        if( y > mThreshold )
        {
            // width
            float width;
            switch( mWidthMode )
            {
                case WIDTH_FIXED:
                    width = mBaseWidth;
                    break;
                    
                case WIDTH_FIXED_FREQ:
                    width = mBaseWidth+(1.0f-x/dataSize)*mMaxWidth;
                    break;
                
                case WIDTH_AUDIO_FREQ:
                    width = mBaseWidth+(1.0f-x/dataSize)*mMaxWidth*y;
                    break;
                    
                default:
                    assert(false && "invalid");
            }
        
            if (width > mLines[index][i].mWidth)
            {
                mLines[index][i].mWidth = width;
                timeline().apply( &mLines[index][i].mWidth, 0.0f, falloff, easeFn );
            }
            width = mLines[index][i].mWidth;
            
            // position
            float lineX = getPositionOffset( index, i, x, y, dataSize, barRange );
            
            // alpha
            ColorA color( mColor.r, mColor.g, mColor.b, mColor.a );
            float alpha = color.a;
            if( mAlphaByFft )
            {
                alpha = mColor.a + y;
                mLines[index][i].mAlpha = alpha;
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

float Barcode::getPositionOffset( const int barIndex, const int lineIndex, const float x, const float y, const float dataSize, const float barRange )
{
    float offset = 0.0f;
    
    switch( mPositionMode )
    {
        case POSITION_FIXED_CENTER:
            if( lineIndex % 2 == 0 )
            {
                offset = (barRange/2.0f) + (x/dataSize)*(barRange/2.0f);
            }
            else
            {
                offset = (barRange/2.0f) - (x/dataSize)*(barRange/2.0f);
            }
            break;
            
        case POSITION_SHIFT_RANDOM:
            offset = Rand::randFloat(0.0f,barRange);
            break;
            
        case POSITION_SHIFT_LINEAR:
            // TODO: store offset as line var
            break;
            
        case POSITION_FIXED_RANDOM:
        {
            // TODO: inefficient
            Rand fixedRand( barIndex );
            for( int i=0; i < barIndex; ++i )
            {
                offset = fixedRand.nextFloat(barRange);
            }
        }
            break;
            
        default:
            assert( false && "unhandle" );
            break;
    }
    
    return offset;
}
