//
//  LissajousShader.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-25.
//
//

#include "LissajousShader.h"

using namespace ci;
using namespace std;

LissajousShader::LissajousShader()
: FragShader("lissajous", "lissajous_frag.glsl")
, mFrequencyX(2.0f)
, mFrequencyY(3.0f)
, mFrequencyXShift(0.0f)
, mFrequencyYShift(0.0f)
, mScale(0.8f)
, mResponseBandX(AudioInputHandler::BAND_NONE)
, mResponseBandY(AudioInputHandler::BAND_NONE)
, mColor(ColorAf(0.2f, 1.0f, 0.1f, 1.0f))
{
    
}

void LissajousShader::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + mName;
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(mName);
    interface->addParam(CreateIntParam( "frequency-x", &mFrequencyX )
                        .minValue(1)
                        .maxValue(32)
                        .oscReceiver(oscName));
    interface->addParam(CreateIntParam( "frequency-y", &mFrequencyY )
                        .minValue(1)
                        .maxValue(32)
                        .oscReceiver(oscName));
    
    interface->addParam(CreateFloatParam( "frequency-x-shift", &mFrequencyXShift )
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam( "frequency-y-shift", &mFrequencyYShift )
                        .oscReceiver(oscName));
    
    interface->addEnum(CreateEnumParam("x-band", &mResponseBandX)
                       .maxValue(bandNames.size())
                       .isVertical()
                       .oscReceiver(oscName)
                       .sendFeedback(), bandNames);
    interface->addEnum(CreateEnumParam("y-band", &mResponseBandY)
                       .maxValue(bandNames.size())
                       .isVertical()
                       .oscReceiver(oscName)
                       .sendFeedback(), bandNames);
    interface->addParam(CreateFloatParam("scale", &mScale)
                        .minValue(0.05f)
                        .maxValue(2.0f)
                        .oscReceiver(oscName));
    interface->addParam(CreateColorParam("color", &mColor, kMinColor, kMaxColor)
                        .oscReceiver(oscName));
}

void LissajousShader::setCustomParams( AudioInputHandler& audioInputHandler )
{
    mShader.uniform("iScale", mScale);
    
    mShader.uniform("iColor", mColor);
    
    float frequencyX = (mFrequencyX + mFrequencyXShift) * audioInputHandler.getAverageVolumeByBand(mResponseBandX);
    float frequencyY = (mFrequencyY + mFrequencyYShift) * audioInputHandler.getAverageVolumeByBand(mResponseBandY);
    
    mShader.uniform("iFrequencyX", frequencyX);
    mShader.uniform("iFrequencyY", frequencyY);
}
