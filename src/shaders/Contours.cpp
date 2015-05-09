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
{

}

void Contours::Marble::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    
}

void Contours::Marble::update(double dt)
{
    
}

void Contours::Marble::setCustomParams( AudioInputHandler& audioInputHandler )
{
    
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
    
}

void Contours::Warping::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    
}

void Contours::Warping::update(double dt)
{
    
}

void Contours::Warping::setCustomParams( AudioInputHandler& audioInputHandler )
{
    
}

