//
//  Cymatics.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-18.
//
//

#include "Cymatics.h"
#include "OculonApp.h"
#include "Utils.h"

using namespace ci;
using namespace std;

Cymatics::Cymatics()
: Scene("cymatics")
{
}

Cymatics::~Cymatics()
{
}

void Cymatics::setup()
{
    Scene::setup();
    
    // params
    mColor1 = ColorAf(1.0f, 1.0f, 0.858824f);
    mColor2 = ColorAf(0.0f, 0.333333f, 1.0f);
    
    mPoles = 50;
    mDistance = 0.25f;
    mShift = 0.0f;
    
    mPolesResponseBand = AudioInputHandler::BAND_MID;
    mDistanceResponseBand = AudioInputHandler::BAND_LOW;
    
    mAudioInputHandler.setup(false);
    
    // TODO: set time controller min/max ranges
    
    mShader = loadFragShader("cymatics_frag.glsl");
    
    reset();
}

void Cymatics::reset()
{
    mTimeController.reset();
}

void Cymatics::setupInterface()
{
    vector<string> bandNames = mAudioInputHandler.getBandNames();
    
    mTimeController.setupInterface(mInterface, mName);
    
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor).oscReceiver(mName));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor).oscReceiver(mName));
    
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateIntParam( "poles", &mPoles )
                         .minValue(2)
                         .maxValue(300));
    mInterface->addEnum(CreateEnumParam( "audio-poles", (int*)(&mPolesResponseBand) )
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(mName)
                        .sendFeedback(), bandNames);

    mInterface->addParam(CreateFloatParam("distance", &mDistance)
                         .minValue(0.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    
    mInterface->addEnum(CreateEnumParam( "audio-distance", (int*)(&mDistanceResponseBand) )
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(mName)
                        .sendFeedback(), bandNames);
    
    mInterface->addParam(CreateFloatParam("shift", &mShift)
                         .minValue(-100.0f)
                         .maxValue(100.0f)
                         .oscReceiver(getName()));
    
    mAudioInputHandler.setupInterface(mInterface, mName);
}

void Cymatics::update(double dt)
{
    Scene::update(dt);
    
    mTimeController.update(dt);
    mAudioInputHandler.update(dt, mApp->getAudioInput());
}

void Cymatics::draw()
{
    gl::pushMatrices();
    
    gl::enableAlphaBlending();
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    gl::pushMatrices();
    gl::setMatricesWindow( mApp->getViewportSize() );
    
    shaderPreDraw();
    
    Utils::drawTexturedRect( mApp->getViewportBounds() );
    
    shaderPostDraw();
    
    gl::popMatrices();
    
    gl::popMatrices();
}

void Cymatics::shaderPreDraw()
{
    mShader.bind();
    
    Vec3f resolution = Vec3f( mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f );
    
    mShader.uniform( "iResolution", resolution );
    mShader.uniform( "iGlobalTime", (float)mTimeController.getElapsedSeconds() );
    mShader.uniform( "iColor1", Color(mColor1) );
    mShader.uniform( "iColor2", Color(mColor2) );
    mShader.uniform( "iShift", mShift );
    
    int poles = mPoles;
    if (mPolesResponseBand != AudioInputHandler::BAND_NONE)
    {
        poles = math<int>::clamp( mPoles * mAudioInputHandler.getAverageVolumeByBand(mPolesResponseBand), 2, 200 );
    }
    mShader.uniform( "iPoles", poles );
    
    float distance = mDistance;
    if (mDistanceResponseBand != AudioInputHandler::BAND_NONE)
    {
        distance = mDistance * mAudioInputHandler.getAverageVolumeByBand(mDistanceResponseBand);
    }
    mShader.uniform( "iDistance", distance );
}

void Cymatics::shaderPostDraw()
{
    mShader.unbind();
}