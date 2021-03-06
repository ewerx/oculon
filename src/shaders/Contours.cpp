//
//  Contours.cpp
//  Oculon1
//
//  Created by Ehsan Rezaie on 2015-05-08.
//
//

#include "Contours.h"

#include "Interface.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

Contours::Contours()
: TextureShaders("contours")
{
    mColor1 = ColorA::white();
    mColor2 = ColorA::black();
    mBackgroundAlpha = 0.0f;
}

Contours::~Contours()
{
}

void Contours::setupShaders()
{
    mShaderType = 0;
    
    mShaders.push_back( new Marble() );
    mShaders.push_back( new Elevation() );
    mShaders.push_back( new Warping() );
}

#pragma mark - Marble

Contours::Marble::Marble()
: FragShader("marble", "marble.frag")
, mDensity(3.0f)
, mZoom(0.1f)
{

}

void Contours::Marble::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
//    interface->addParam(CreateFloatParam( "scale", &mDensity )
//                        .minValue(1.1f)
//                        .maxValue(16.0f));
    interface->addParam(CreateFloatParam( "zoom", &mZoom )
                        .minValue(0.001f)
                        .maxValue(0.5f));
}

void Contours::Marble::update(double dt)
{
    
}

void Contours::Marble::setCustomParams( AudioInputHandler& audioInputHandler )
{
    mShader.uniform("iDensity", mDensity);
    mShader.uniform("iZoom", mZoom);
}

#pragma mark - Elevation

Contours::Elevation::Elevation()
: FragShader("elevation", "elevation.frag")
{
    
}

void Contours::Elevation::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    
}

void Contours::Elevation::update(double dt)
{
    
}

void Contours::Elevation::setCustomParams( AudioInputHandler& audioInputHandler )
{
    
}

#pragma mark - Warping

Contours::Warping::Warping()
: FragShader("warping", "warping_frag.glsl")
{
    mLevels = 1.25f;
    mZoom = 6.0f;
    mRefraction = 2.0f;
}

void Contours::Warping::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    
    interface->addParam(CreateFloatParam("Levels", &mLevels)
                        .minValue(0.0f)
                        .maxValue(20.0f));
    interface->addParam(CreateFloatParam("Zoom", &mZoom)
                        .minValue(1.0f)
                        .maxValue(24.0f));
    interface->addParam(CreateFloatParam("Refraction", &mRefraction)
                        .minValue(1.5f)
                        .maxValue(9.0f));
    
}

void Contours::Warping::update(double dt)
{
    
}

void Contours::Warping::setCustomParams( AudioInputHandler& audioInputHandler )
{
    mShader.uniform("iLevels", mLevels);
    mShader.uniform("iZoom", mZoom);
    mShader.uniform("iRefraction", mRefraction);
}

