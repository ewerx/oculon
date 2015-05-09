//
//  Tilings.cpp
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

#include "Interface.h"
#include "OculonApp.h"
#include "Tilings.h"
#include "Utils.h"
#include "cinder/Rand.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace std;

Tilings::Tilings()
: TextureShaders("tilings")
{
    mBackgroundAlpha = 0.0f;
    mColor1 = ColorA::white();
    mColor2 = ColorA::black();
    mTimeController.setTimeScale(0.05f);
}

Tilings::~Tilings()
{
}

void Tilings::setupShaders()
{
    mShaderType = 0;
    
    mShaders.push_back( new Tessellations() );
    mShaders.push_back( new Voronoi() );
}

#pragma mark - Tessellations

Tilings::Tessellations::Tessellations()
: FragShader("tessellations", "tilings_frag.glsl")
{
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

void Tilings::Tessellations::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    interface->addParam(CreateFloatParam( "h-offset", &mHOffset )
                         .minValue(-0.5f)
                         .maxValue(0.5f));
    interface->addParam(CreateFloatParam( "scale", &mScale )
                         .minValue(0.0f)
                         .maxValue(1.0f));
    
    interface->addParam(CreateIntParam( "iterations", &mIterations )
                         .minValue(1)
                         .maxValue(40)
                         .oscReceiver(getName()));
    interface->addParam(CreateIntParam( "anglep", &mAngleP )
                         .minValue(2)
                         .maxValue(40)
                         .oscReceiver(getName()));
    interface->addEnum(CreateEnumParam("p-response", &mAnglePResponse)
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(getName())
                        .sendFeedback(), bandNames);
    interface->addParam(CreateIntParam( "angleq", &mAngleQ )
                         .minValue(2)
                         .maxValue(12)
                         .oscReceiver(getName()));
    interface->addEnum(CreateEnumParam("q-response", &mAngleQResponse)
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(getName())
                        .sendFeedback(), bandNames);
    interface->addParam(CreateIntParam( "angler", &mAngleR )
                         .minValue(1)
                         .maxValue(12)
                         .oscReceiver(getName()));
    interface->addEnum(CreateEnumParam("r-response", &mAngleRResponse)
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(getName())
                        .sendFeedback(), bandNames);
    
    interface->addEnum(CreateEnumParam("offset-response", &mOffsetResponse)
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(getName())
                        .sendFeedback(), bandNames);
    
    interface->addParam(CreateFloatParam( "Thickness", &mThickness )
                         .minValue(0.0f)
                         .maxValue(1.0f));
}

void Tilings::Tessellations::update(double dt)
{
}

void Tilings::Tessellations::setCustomParams( AudioInputHandler& audioInputHandler )
{
    int angleP = mAngleP * (0.5f + audioInputHandler.getAverageVolumeByBand(mAnglePResponse));
    angleP = math<int>::clamp( angleP, 1, 40 );
    
    int angleQ = mAngleQ * (0.5f + audioInputHandler.getAverageVolumeByBand(mAngleQResponse));
    angleQ = math<int>::clamp( angleQ, 1, 40 );
    
    int angleR = mAngleR * (0.5f + audioInputHandler.getAverageVolumeByBand(mAngleRResponse));
    angleR = math<int>::clamp( angleR, 1, 40 );
    
    float offset = audioInputHandler.getAverageVolumeByBand(mOffsetResponse);
    
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

#pragma mark - Voronoi

Tilings::Voronoi::Voronoi()
: FragShader("voronoi", "voronoi_frag.glsl")
{
    mBorderColor = Vec3f( 1.0f, 1.0f, 1.0f );
    mZoom = 60.0f;
    mBorderIn = 0.0f;
    mBorderOut = 0.075f;
    mSeedColor = Vec3f( 1.0f, 1.0f, 1.0f );
    mSeedSize = 0.0f;
    mCellLayers = 8.0f;
    mCellBrightness = 0.5f;
    mCellBorderStrength = 0.5f;
    mCellColor = Vec3f( 0.0f, 0.0f, 0.0f );
    mSpeed = 1.00f;
    mDistortion = 0.0f;
    mAudioDistortion = true;
    
    mAudioOffset = false;
    mResponseBand = 1;
}

void Tilings::Voronoi::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    interface->addParam(CreateFloatParam("voronoi/speed", &mSpeed)
                         .maxValue(10.0f)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam("voronoi/zoom", &mZoom)
                         .maxValue(256.0f)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam("voronoi/distortion", &mDistortion)
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    interface->gui()->addSeparator();
    interface->addParam(CreateVec3fParam("voronoi/line_color", &mBorderColor, Vec3f::zero(), Vec3f(3.0f,3.0f,3.0f)));
    interface->addParam(CreateFloatParam("voronoi/borderin", &mBorderIn)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam("voronoi/borderout", &mBorderOut)
                         .oscReceiver(getName()));
    interface->gui()->addSeparator();
    interface->addParam(CreateVec3fParam("voronoi/seed_color", &mSeedColor, Vec3f::zero(), Vec3f(3.0f,3.0f,3.0f)));
    interface->addParam(CreateFloatParam("voronoi/seedsize", &mSeedSize)
                         .oscReceiver(getName()));
    interface->gui()->addSeparator();
    interface->addParam(CreateVec3fParam("voronoi/cell_color", &mCellColor, Vec3f::zero(), Vec3f(3.0f,3.0f,3.0f)));
    interface->addParam(CreateFloatParam("voronoi/cell_brightness", &mCellBrightness)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam("voronoi/cell_strength", &mCellBorderStrength)
                         .oscReceiver(getName()));
    
    interface->addParam(CreateBoolParam("voronoi/audio_distortion", &mAudioDistortion)
                         .oscReceiver(getName()));
    interface->addEnum(CreateEnumParam("audioband", &mResponseBand)
                       .maxValue(bandNames.size())
                       .isVertical()
                       .oscReceiver(getName())
                       .sendFeedback(), bandNames);
}

void Tilings::Voronoi::update(double dt)
{
}

void Tilings::Voronoi::setCustomParams( AudioInputHandler& audioInputHandler )
{
    mShader.uniform( "borderColor", mBorderColor );
    mShader.uniform( "zoom", mZoom );
    mShader.uniform( "speed", mSpeed );
    mShader.uniform( "borderIn", mBorderIn );
    mShader.uniform( "borderOut", mBorderOut );
    mShader.uniform( "seedSize", mSeedSize );
    mShader.uniform( "seedColor", mSeedColor );
    mShader.uniform( "cellLayers", mCellLayers );
    mShader.uniform( "cellColor", mCellColor );
    mShader.uniform( "cellBorderStrength", mCellBorderStrength );
    mShader.uniform( "cellBrightness", mCellBrightness );
    
    float distortion = mDistortion;
    if (mAudioDistortion)
    {
        float audioLevel = 0.0f;
        switch (mResponseBand) {
            case 0:
            audioLevel = audioInputHandler.getAverageVolumeLowFreq();
            break;
            case 1:
            audioLevel = audioInputHandler.getAverageVolumeMidFreq();
            break;
            case 2:
            audioLevel = audioInputHandler.getAverageVolumeHighFreq();
            break;
            default:
            break;
        }
        distortion += 10.0f * audioLevel;
        distortion = math<float>::min(distortion, 1.0f);
    }
    mShader.uniform( "distortion", distortion );
}

