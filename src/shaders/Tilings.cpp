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

Tilings::Tilings(const std::string& name)
: TextureShaders(name)
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
//    mShaders.push_back( new VoronoiPlasma() );
    mShaders.push_back( new VoronoiFire() );
    mShaders.push_back( new VoronoiCells() );
    
    if (getName().compare("voronoi-fire") == 0)
    {
        mShaderType = 2;
    }
}

#pragma mark - Tessellations

Tilings::Tessellations::Tessellations()
: FragShader("tessellations", "tilings_frag.glsl")
, mOffset(getName(), 0.0f, 0.0f, 1.0f, true)
, mAngleP(getName(), 3.0f, 2.0f, 40.0f, true)
, mAngleQ(getName(), 5.0f, 2.0f, 12.0f, true)
, mAngleR(getName(), 2.0f, 1.0f, 12.0f, true)
{
    mHOffset = 0.0f;
    mScale = 0.47619f;
    
    mIterations = 20;
    mThickness = 0.03f;
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
    
    mAngleP.setupInterface(interface, "angle-p");
    mAngleQ.setupInterface(interface, "angle-q");
    mAngleR.setupInterface(interface, "angle-r");
    mOffset.setupInterface(interface, "offset");
    
    interface->addParam(CreateFloatParam( "Thickness", &mThickness )
                         .minValue(0.0f)
                         .maxValue(1.0f));
}

void Tilings::Tessellations::update(double dt)
{
}

void Tilings::Tessellations::setCustomParams( AudioInputHandler& audioInputHandler )
{
    int angleP = mAngleP(audioInputHandler);
    int angleQ = mAngleQ(audioInputHandler);
    int angleR = mAngleR(audioInputHandler);
    float offset = mOffset(audioInputHandler);
    
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

#pragma mark - Liquid Cubes

Tilings::VoronoiPlasma::VoronoiPlasma()
: FragShader("voronoi-plasma", "voronoi_plasma_frag.glsl")
{
}

void Tilings::VoronoiPlasma::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    
    
}

void Tilings::VoronoiPlasma::setCustomParams(AudioInputHandler &audioInputHandler)
{
}

#pragma mark - Liquid Cubes

Tilings::VoronoiFire::VoronoiFire()
: FragShader("voronoi-fire", "voronoi_fire_frag.glsl")
, mAmount(getName(), 2.0f, 0.001f, 8.0f, true)
, mAmpScale(getName(), 0.5f, 0.0f, 1.0f, true)
, mScale(getName(), 2.0f, 1.0f, 16.0f, true)
, mContrast(getName(), 1.5f, 0.5f, 5.0f, true)
{
    mLayers = 5;
//    mAmount = 2.0;
    mTurbulence = 1.5;
//    mAmpScale = 0.5;
    mShiftRate = Vec2f(0.25, 0.125);
    mSpinRate = 0.25;
//    mScale = 2.0;
    mNoise = 0.015;
//    mContrast = 1.5;
}

void Tilings::VoronoiFire::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    
    interface->addParam(CreateIntParam("v-fire/Layers", &mLayers)
                        .minValue(1)
                        .maxValue(6)
                        .oscReceiver(getName()));
    mAmount.setupInterface(interface, "amount");
    mAmpScale.setupInterface(interface, "amp-scale");
    mScale.setupInterface(interface, "scale");
//    interface->addParam(CreateFloatParam("v-fire/Amount", &mAmount)
//                        .minValue(0.001f)
//                        .maxValue(8.0f)
//                        .oscReceiver(getName()));
//    interface->addParam(CreateFloatParam("v-fire/AmpScale", &mAmpScale)
//                        .minValue(0.0f)
//                        .maxValue(1.0f)
//                        .oscReceiver(getName()));
//    interface->addParam(CreateFloatParam("v-fire/Scale", &mScale)
//                        .minValue(1.0f)
//                        .maxValue(16.0f)
//                        .oscReceiver(getName()));
    interface->addParam(CreateFloatParam("v-fire/Turbulence", &mTurbulence)
                        .minValue(0.0f)
                        .maxValue(8.0f)
                        .oscReceiver(getName()));
    interface->addParam(CreateFloatParam("v-fire/SpinRate", &mSpinRate)
                        .minValue(0.0f)
                        .maxValue(10.0f)
                        .oscReceiver(getName()));
    interface->addParam(CreateFloatParam("v-fire/Noise", &mNoise)
                        .minValue(0.0f)
                        .maxValue(0.1f)
                        .oscReceiver(getName()));
//    interface->addParam(CreateFloatParam("v-fire/Contrast", &mContrast)
//                        .minValue(0.5f)
//                        .maxValue(5.0f)
//                        .oscReceiver(getName()));
    mContrast.setupInterface(interface, "contrast");
}

void Tilings::VoronoiFire::setCustomParams(AudioInputHandler &audioInputHandler)
{
    mShader.uniform( "iLayers", mLayers );
    mShader.uniform( "iAmount", mAmount(audioInputHandler) );
    mShader.uniform( "iTurbulence", mTurbulence );
    mShader.uniform( "iAmpScale", mAmpScale(audioInputHandler) );
    mShader.uniform( "iShiftRate", mShiftRate );
    mShader.uniform( "iSpinRate", mSpinRate );
    mShader.uniform( "iScale", mScale(audioInputHandler) );
    mShader.uniform( "iNoise", mNoise );
    mShader.uniform( "iContrast", mContrast(audioInputHandler) );
}

#pragma mark - Liquid Cubes

Tilings::VoronoiCells::VoronoiCells()
: FragShader("voronoi-cells", "voronoi_cells_frag.glsl")
{
    mGap = 0.25;
    mAgitation = 2.0;
    mZoom = 10.0;
}

void Tilings::VoronoiCells::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    
    interface->addParam(CreateFloatParam("v-cells/gap", &mGap)
                        .maxValue(0.5f)
                        .oscReceiver(getName()));
    interface->addParam(CreateFloatParam("v-cells/agitation", &mAgitation)
                        .maxValue(10.0f)
                        .oscReceiver(getName()));
    interface->addParam(CreateFloatParam("v-cells/zoom", &mZoom)
                        .maxValue(30.0f)
                        .oscReceiver(getName()));
}

void Tilings::VoronoiCells::setCustomParams(AudioInputHandler &audioInputHandler)
{
    mShader.uniform( "iZoom", mZoom );
    mShader.uniform( "iAgitation", mAgitation );
    mShader.uniform( "iGap", mGap );
}
