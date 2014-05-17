/*
 *  Circles.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */


#include "Circles.h"
#include "Utils.h"
#include "Scene.h"
#include "OculonApp.h"
#include "Interface.h"
#include "Constants.h"
#include "Resources.h"

#include "cinder/gl/gl.h"
#include "cinder/CinderMath.h"
#include "cinder/Rand.h"
#include "cinder/Easing.h"
#include "cinder/ObjLoader.h"

using namespace ci;
using namespace ci::app;
using namespace std;


//
// Circles
// 

Circles::Circles(Scene* scene)
: SubScene(scene,"circles")
{
}

Circles::~Circles()
{
}

void Circles::setup()
{
    // params
    mLayers = 1;
    mSpacing = 0;
    mBaseRadius = 8.0f;
    mMinRadiusFactor = 0.25f;
    
    mSignalScale = 0.16f;
    mFalloff = 2.0f;
    mFftFalloff = true;
    mAlphaByAudio = true;
    mFalloffMode = FALLOFF_OUTBOUNCE;
    mShape = SHAPE_CIRCLE;
    
    mShiftSpeedX = 0.0f;
    mShiftSpeedY = 0.0f;
    mShiftAlternateX = true;
    mShiftAlternateY = false;
    
    mColorPrimary = ColorA(1.0f,1.0f,1.0f,1.0f);
    mColorSecondary = ColorA(1.0f,1.0f,1.0f,1.0f);
    
    for (int i=0; i < MAX_X; ++i)
    {
        for (int j=0; j < MAX_Y; ++j)
        {
            mCircles[i][j].mRadius = 0.0f;
            mCircles[i][j].mAlpha = 0.0f;
        }
    }
    
    reset();
}

void Circles::setupInterface()
{
    SubScene::setupInterface();
    
    Interface* interface = mParentScene->getInterface();
    
    interface->addParam(CreateIntParam("Rows", &mRows)
                        .minValue(1)
                        .maxValue(MAX_Y)
                        .oscReceiver(mName,"rows")
                        .sendFeedback());
    interface->addParam(CreateIntParam("Cols", &mCols)
                        .minValue(1)
                        .maxValue(MAX_X)
                        .oscReceiver(mName,"cols")
                        .sendFeedback());
    interface->addParam(CreateFloatParam("Base Radius", &mBaseRadius)
                        .minValue(1.0f)
                        .maxValue(200.0f)
                        .oscReceiver(mName,"radius"))->registerCallback( this, &Circles::baseRadiusChanged );
    interface->addEnum(CreateEnumParam("Shape", (int*)&mShape)
                       .maxValue(SHAPE_COUNT)
                       .oscReceiver(mName,"shape")
                       .isVertical());
//    interface->addParam(CreateFloatParam("Base Alpha", &mBaseAlpha)
//                        .oscReceiver(mName,"alpha"));
    interface->addParam(CreateFloatParam("Falloff", &mFalloff)
                        .minValue(0.0f)
                        .maxValue(20.0f));
    interface->addParam(CreateBoolParam("FFT Falloff", &mFftFalloff));
    interface->addEnum(CreateEnumParam("Falloff Mode", (int*)&mFalloffMode)
                       .maxValue(FALLOFF_COUNT)
                       .oscReceiver(mName,"falloffmode")
                       .isVertical());
    interface->addParam(CreateIntParam("Spacing", &mSpacing)
                        .minValue(0)
                        .maxValue(100)
                        .oscReceiver(mName,"spacing"))->registerCallback( this, &Circles::baseRadiusChanged );
    interface->addParam(CreateFloatParam("Signal Scale", &mSignalScale)
                        .minValue(0.0f)
                        .maxValue(10.0f)
                        .oscReceiver(mName,"signalscale")
                        .sendFeedback());
    interface->addParam(CreateColorParam("Color 1", &mColorPrimary, kMinColor, kMaxColor)
                        .oscReceiver(mName,"color1"));
    interface->addParam(CreateColorParam("Color 2", &mColorSecondary, kMinColor, kMaxColor)
                        .oscReceiver(mName,"color2"));
    
    // shift speed
    // TODO: XY pad
    // TODO: centered faders
    interface->addParam(CreateFloatParam("Shift X", &mShiftSpeedX)
                        .minValue(-10.0f)
                        .maxValue(10.0f)
                        .oscReceiver(mName,"shiftx")
                        .sendFeedback());
    interface->addParam(CreateFloatParam("Shift Y", &mShiftSpeedY)
                        .minValue(-1.0f)
                        .maxValue(1.0f)
                        .oscReceiver(mName,"shifty")
                        .sendFeedback());
    interface->addParam(CreateBoolParam("Alt Shift X", &mShiftAlternateX));
    interface->addParam(CreateBoolParam("Alt Shift Y", &mShiftAlternateY));
    
}

//void Circles::setupDebugInterface()
//{
//}

void Circles::reset()
{
    const float windowHeight = mParentScene->getApp()->getViewportHeight();
    const float windowWidth = mParentScene->getApp()->getViewportWidth();
    
    mCols = math<int>::min( (windowWidth / ((float)mSpacing + mBaseRadius*2.0f)), MAX_X );
    mRows = math<int>::min( (windowHeight / ((float)mSpacing + mBaseRadius*2.0f)), MAX_Y );
    
    mCols = 72;
    mRows = 72;
    
    for( int col = 0; col < mCols; ++col )
    {
        for( int row = 0; row < mRows; ++row )
        {
            mCircles[col][row].mRadius = mBaseRadius * mMinRadiusFactor;
            //mCircles[col][row].mAlpha = mColorPrimary.a;
            mCircles[col][row].mPos = Vec2f::zero();
        }
    }
}

void Circles::resize()
{
}

void Circles::update(double dt)
{
}

void Circles::draw()
{
    // audio data
    AudioInput& audioInput = mParentScene->getApp()->getAudioInput();
    //float * freqData = audioInput.getFft()->getAmplitude();
    //float * timeData = audioInput.getFft()->getData();
//    int32_t dataSize = audioInput.getFft()->getBinSize();
//    const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
    // dimensions
    const float windowHeight = mParentScene->getApp()->getViewportHeight();
    const float windowWidth = mParentScene->getApp()->getViewportWidth();
    
    gl::pushMatrices();
    
    float totalWidth = mBaseRadius * 2.0f * mCols + (float)mSpacing * (mCols-1);
    float totalHeight = mBaseRadius * 2.0f * mRows + (float)mSpacing * (mRows-1);
    
    float x0 = (windowWidth - totalWidth) / 2.0f + mBaseRadius;
    float y0 = (windowHeight - totalHeight) / 2.0f + mBaseRadius;

    float x = x0;
    float y = y0;
    
    //int midRow = mRows / 2;
    int midCol = mCols / 2;
    
    //bool evenRows = (mRows % 2) == 0;
    //bool evenCols = (mCols % 2) == 0;
    
    float maxRadius = mBaseRadius + (float)mSpacing - 2.0f;
    const float overlapFactor = 1.1f;
    
    glLineWidth( 2.0f );
    glEnable(GL_LINE_SMOOTH);
    
    for( int col = 0; col < mCols; ++col )
    {
        for( int row = 0; row < mRows; ++row )
        {
            float multiplier = 1.0f;
            
            if( (mShiftAlternateX && row % 2 == 0) )
            {
                multiplier = -1.0f;
            }
            
            mCircles[col][row].mPos.x += mShiftSpeedX * multiplier;
            //mCircles[col][row].mPos.y += mShiftSpeedY * multiplier;
            
            if (mCircles[col][row].mPos.x >= (mBaseRadius * 2.0f + mSpacing) ||
                mCircles[col][row].mPos.x <= -(mBaseRadius * 2.0f + mSpacing))
            {
                mCircles[col][row].mPos.x = 0.0f;
            }
            if (mCircles[col][row].mPos.y >= (mBaseRadius * 2.0f + mSpacing) ||
                mCircles[col][row].mPos.y <= -(mBaseRadius * 2.0f + mSpacing))
            {
                mCircles[col][row].mPos.y = 0.0f;
            }
            
//            int fftIndex = 0;
//            if( 1 ) // random signal - HACK
//            {
//                fftIndex = math<int>::max(2,Rand::randInt(dataSize));
//            }
//            else
//            {
//                fftIndex = math<int>::abs(midCol-col) + 2;
//            }
//            float falloff = mFftFalloff ? (mFalloff * (1.0f - fftIndex / dataSize)) : mFalloff;
//            
//            float radius = math<float>::max(mBaseRadius * mMinRadiusFactor,fftLogData[fftIndex].y * maxRadius * (1+fftIndex * mSignalScale));
//            radius = math<float>::min(radius, maxRadius * overlapFactor);
//            
//            if (radius > mCircles[col][row].mRadius) {
//                mCircles[col][row].mRadius = radius;
//                timeline().apply( &mCircles[col][row].mRadius, 0.0f, falloff, getFalloffFunction() );
//            }
            
            float alpha = mColorPrimary.a;
            
//            if (mAlphaByAudio)
//            {
//                alpha = math<float>::max( mColorPrimary.a, (radius-mBaseRadius) / (maxRadius*overlapFactor-mBaseRadius) );
//                if (mCircles[col][row].mAlpha < alpha )
//                {
//                    mCircles[col][row].mAlpha = alpha;
//                    timeline().apply( &mCircles[col][row].mAlpha, 0.0f, falloff, getFalloffFunction() );
//                }
//            }
            
            gl::color( mColorPrimary.r, mColorPrimary.g, mColorPrimary.b, mCircles[col][row].mAlpha );
            
            //console() << "fftIndex " << fftIndex << " = " << fftLogData[fftIndex].y << std::endl;
            
            Vec2f pos( x + mCircles[col][row].mPos.x, y + mCircles[col][row].mPos.y );
            
            switch (mShape)
            {
                case SHAPE_CIRCLE:
                    gl::drawSolidCircle(pos, mCircles[col][row].mRadius);
                    break;
                    
                case SHAPE_RING:
                    gl::drawStrokedCircle(pos, mCircles[col][row].mRadius);
                    break;
                    
                case SHAPE_SOLID_SQUARE:
                    gl::drawSolidRect( Rectf(pos.x-mCircles[col][row].mRadius, pos.y-mCircles[col][row].mRadius, pos.x+mCircles[col][row].mRadius, pos.y+mCircles[col][row].mRadius) );
                    break;
                    
                case SHAPE_STROKED_SQUARE:
                    gl::drawStrokedRect( Rectf(pos.x-mCircles[col][row].mRadius, pos.y-mCircles[col][row].mRadius, pos.x+mCircles[col][row].mRadius, pos.y+mCircles[col][row].mRadius) );
                    break;
                    
                default:
                    break;
            }
            
            y += mBaseRadius * 2.0f + (float)mSpacing;
        }
        
        y = y0;
        x += mBaseRadius * 2.0f + (float)mSpacing;
    }
    
    //console() << "--------------------------\n";
    
    gl::popMatrices();

}

bool Circles::baseRadiusChanged()
{
    reset();
    return false;
}

Circles::tEaseFn Circles::getFalloffFunction()
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
