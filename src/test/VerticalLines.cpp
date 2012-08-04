/*
 *  VerticalLines.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */


#include "VerticalLines.h"
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
// VerticalLines
// 

VerticalLines::VerticalLines(Scene* scene)
: mParentScene(scene)
{
}

VerticalLines::~VerticalLines()
{
}

void VerticalLines::setup()
{
    mColor = Colorf::white();
    
    mBaseWidth = 1.0f;
    mMaxWidth = 15.0f;
    mIntensity = 0.25f;
    mThreshold = 0.02f;
    mWidthByFft = true;
    mAlphaByFft = true;
    mPositionByFft = false;
    
    mFftMode = FFT_ALPHA;
    
    reset();
}

void VerticalLines::setupInterface()
{
    Interface* interface = mParentScene->getInterface();
    const string name("lines");
    interface->gui()->addColumn();
    interface->gui()->addLabel("Vertical Lines");
    interface->addParam(CreateBoolParam("Width By Audio", &mWidthByFft)
                        .oscReceiver(name,"widthaudio"));
    interface->addParam(CreateBoolParam("Alpha By Audio", &mAlphaByFft)
                        .oscReceiver(name,"alphaaudio"));
    interface->addParam(CreateBoolParam("Pos By Audio", &mPositionByFft)
                        .oscReceiver(name,"posaudio"));
    interface->addParam(CreateFloatParam("Base Width", &mBaseWidth)
                        .minValue(1.0f)
                        .maxValue(10.0f)
                        .oscReceiver(name,"basewidth"));
    interface->addParam(CreateFloatParam("Max Width", &mMaxWidth)
                        .minValue(1.0f)
                        .maxValue(50.0f)
                        .oscReceiver(name,"maxwidth"));
    interface->addParam(CreateFloatParam("Intensity", &mIntensity)
                        .oscReceiver(name,"intensity"));
    interface->addParam(CreateFloatParam("Threshold", &mThreshold)
                        .maxValue(0.1f)
                        .oscReceiver(name,"threshold"));
    interface->addParam(CreateColorParam("Color", &mColor, kMinColor, kMaxColor)
                        .oscReceiver(name,"color"));
}

void VerticalLines::setupDebugInterface()
{
}

void VerticalLines::reset()
{
}

void VerticalLines::resize()
{
}

void VerticalLines::update(double dt)
{
}

void VerticalLines::draw()
{
    AudioInput& audioInput = mParentScene->getApp()->getAudioInput();
	
    // Get data
    //float * freqData = audioInput.getFft()->getAmplitude();
    //float * timeData = audioInput.getFft()->getData();
    int32_t dataSize = audioInput.getFft()->getBinSize();
    const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
    float screenWidth = mParentScene->getApp()->getViewportWidth();
    float screenHeight = mParentScene->getApp()->getViewportHeight();
    
    gl::pushMatrices();
    gl::enableAdditiveBlending();
    // Iterate through data
    for (int32_t i = 0; i < dataSize; i++)
    {
        float x = fftLogData[i].x;
        float y = fftLogData[i].y;
        
        const float minAlpha = 0.25f;
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
        
            float barX;
            if( mPositionByFft )
            {
                
                if( Rand::randFloat() < 0.5f )
                {
                    barX = (screenWidth/2.0f) + (x/dataSize)*(screenWidth/2.0f);
                }
                else
                {
                    barX = (screenWidth/2.0f) - (x/dataSize)*(screenWidth/2.0f);
                }
            }
            else
            {
                barX = Rand::randFloat(0.0f,screenWidth);
            }
            
            ColorA color( mColor.r, mColor.g, mColor.b, mIntensity );
            if( mAlphaByFft )
            {
                color.a = mIntensity + y;
            }
            
            gl::color( color );
            
            glBegin( GL_QUADS );
            // bottom
            glVertex2f( barX, 0.0f );
            glVertex2f( barX + width, 0.0f );
            // top
            glVertex2f( barX + width, screenHeight );
            glVertex2f( barX, screenHeight );
            glEnd();
        }
	}
    gl::enableAlphaBlending();
    gl::popMatrices();

}
