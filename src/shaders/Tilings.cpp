//
//  Tilings.cpp
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

#include "OculonApp.h"
#include "Tilings.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include <boost/format.hpp>
#include "Interface.h"
#include "Utils.h"

using namespace ci;
using namespace ci::app;
using namespace std;

Tilings::Tilings()
: Scene("tilings")
{
}

Tilings::~Tilings()
{
}

void Tilings::setup()
{
    Scene::setup();
    
    mAudioInputHandler.setup(false);
    
    mShader = loadFragShader("tilings_frag.glsl");
    
    mBackgroundAlpha = 0.0f;
    
    mTimeController.setTimeScale(0.2f);
    
    reset();
}

void Tilings::reset()
{
    mColor1 = ColorA::white();
    mColor2 = ColorA::black();
    mOffset = 0.0f;
    mHOffset = 0.0f;
    mScale = 0.47619f;
    
    mIterations = 20;
    mAngleP = 3;
    mAngleQ = 5;
    mAngleR = 2;
    mThickness = 0.03f;
     
    mOffsetResponse = AudioInputHandler::BAND_NONE;
    mAnglePResponse = AudioInputHandler::BAND_NONE;
    mAngleQResponse = AudioInputHandler::BAND_NONE;
    mAngleRResponse = AudioInputHandler::BAND_NONE;
}

void Tilings::setupInterface()
{
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    mTimeController.setupInterface(mInterface, getName());
    
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor));
    
    mInterface->addParam(CreateFloatParam( "h-offset", &mHOffset )
                         .minValue(-0.5f)
                         .maxValue(0.5f));
    mInterface->addParam(CreateFloatParam( "scale", &mScale )
                         .minValue(0.0f)
                         .maxValue(1.0f));
    
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateIntParam( "iterations", &mIterations )
                         .minValue(1)
                         .maxValue(40)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateIntParam( "anglep", &mAngleP )
                         .minValue(2)
                         .maxValue(40)
                         .oscReceiver(getName()));
    mInterface->addEnum(CreateEnumParam("p-response", &mAnglePResponse)
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(getName())
                        .sendFeedback(), bandNames);
    mInterface->addParam(CreateIntParam( "angleq", &mAngleQ )
                         .minValue(2)
                         .maxValue(12)
                         .oscReceiver(getName()));
    mInterface->addEnum(CreateEnumParam("q-response", &mAngleQResponse)
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(getName())
                        .sendFeedback(), bandNames);
    mInterface->addParam(CreateIntParam( "angler", &mAngleR )
                         .minValue(1)
                         .maxValue(12)
                         .oscReceiver(getName()));
    mInterface->addEnum(CreateEnumParam("r-response", &mAngleRResponse)
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(getName())
                        .sendFeedback(), bandNames);
    
    mInterface->addEnum(CreateEnumParam("offset-response", &mOffsetResponse)
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(getName())
                        .sendFeedback(), bandNames);
    
    mInterface->addParam(CreateFloatParam( "Thickness", &mThickness )
                         .minValue(0.0f)
                         .maxValue(1.0f));
    
    mAudioInputHandler.setupInterface(mInterface, getName());
}

void Tilings::update(double dt)
{
    Scene::update(dt);
    
    mTimeController.update(dt);
    mAudioInputHandler.update(dt, mApp->getAudioInput());
}

void Tilings::draw()
{
    gl::pushMatrices();

    drawScene();
    
    gl::popMatrices();
}

void Tilings::shaderPreDraw()
{
    mShader.bind();
    
    Vec2f resolution = Vec2f( mApp->getViewportWidth(), mApp->getViewportHeight() );
    
    int angleP = mAngleP * (0.5f + mAudioInputHandler.getAverageVolumeByBand(mAnglePResponse));
    angleP = math<int>::clamp( angleP, 1, 40 );
    
    int angleQ = mAngleQ * (0.5f + mAudioInputHandler.getAverageVolumeByBand(mAngleQResponse));
    angleQ = math<int>::clamp( angleQ, 1, 40 );
    
    int angleR = mAngleR * (0.5f + mAudioInputHandler.getAverageVolumeByBand(mAngleRResponse));
    angleR = math<int>::clamp( angleR, 1, 40 );
    
    float offset = mAudioInputHandler.getAverageVolumeByBand(mOffsetResponse);
    
    mShader.uniform( "iResolution", resolution );
    mShader.uniform( "iGlobalTime", (float)mTimeController.getElapsedSeconds() );
    
    mShader.uniform( "iTimeScale", mTimeController.getTimeScale() );
    mShader.uniform( "iColor1", mColor1 );
    mShader.uniform( "iColor2", mColor2 );
    mShader.uniform( "iIterations", mIterations );
    mShader.uniform( "iAngleP", angleP );
    mShader.uniform( "iAngleQ", angleQ );
    mShader.uniform( "iAngleR", angleR );
    mShader.uniform( "iCenter", mCenter );
    mShader.uniform( "iThickness", mThickness );
    mShader.uniform( "iOffset", offset);
    mShader.uniform( "iHOffset", mHOffset);
    mShader.uniform( "iScale", mScale);
}

void Tilings::shaderPostDraw()
{
    mShader.unbind();
}

void Tilings::drawScene()
{
    gl::enableAlphaBlending();
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    gl::pushMatrices();
    gl::setMatricesWindow( mApp->getViewportSize() );
    
    shaderPreDraw();
    
    Utils::drawTexturedRect( mApp->getViewportBounds() );
    
    shaderPostDraw();
    
    gl::popMatrices();
}

void Tilings::drawDebug()
{
    //mAudioInputHandler.drawDebug(mApp->getViewportSize());
}
