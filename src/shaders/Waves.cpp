//
//  Waves.cpp
//  Oculon1
//
//  Created by Ehsan Rezaie on 2015-05-05.
//
//

#include "OculonApp.h"
#include "Waves.h"

#include "Interface.h"
#include "Utils.h"

#include "cinder/Utilities.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

Waves::Waves()
: TextureShaders("waves")
{
}

Waves::~Waves()
{
}

void Waves::setupShaders()
{
    mShaderType = 0;
    
    mShaders.push_back( new MultiWave() );
    mShaders.push_back( new SparkWave() );
//    mShaders.push_back( new SineWave() );
//    mShaders.push_back( new Oscilloscope() );
}

#pragma mark - MultiWave

Waves::MultiWave::MultiWave()
: FragShader("multiwave", "multiwave.frag")
{
    
}

void Waves::MultiWave::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
}

void Waves::MultiWave::update(double dt)
{
    
}

void Waves::MultiWave::setCustomParams( AudioInputHandler& audioInputHandler )
{
}


#pragma mark - SparkWave

Waves::SparkWave::SparkWave()
: FragShader("SparkWave", "sparkwave.frag")
{
    
}

void Waves::SparkWave::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
}

void Waves::SparkWave::update(double dt)
{
    
}

void Waves::SparkWave::setCustomParams( AudioInputHandler& audioInputHandler )
{
}
