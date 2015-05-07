//
//  CircleWave.cpp
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

#include "OculonApp.h"
#include "CircleWave.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include <boost/format.hpp>
#include "Interface.h"
#include "Utils.h"

using namespace ci;
using namespace ci::app;
using namespace std;

CircleWave::CircleWave()
: TextureShaders("circlewave")
{
    mColor1 = ColorA::white();
    mColor2 = ColorA::black();
    mBackgroundAlpha = 0.0f;
}

CircleWave::~CircleWave()
{
}

void CircleWave::setupShaders()
{
    mShaderType = 0;
    
    mShaders.push_back( new Circle() );
    mShaders.push_back( new Spark() );
}

#pragma mark - Circle

CircleWave::Circle::Circle()
: FragShader("circle", "circlewave_frag.glsl")
{
    mSeparation = 0.06f;
    mColorSeparation = false;
    mDetail = 0.5f;
    mStrands = 30;
    mScale = 0.75f;
    mThickness = 0.0625f;
    
    //    mBackgroundReaction = BG_REACTION_NONE;
    mBackgroundFlash = false;
}

void CircleWave::Circle::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    interface->addParam(CreateFloatParam( "Separation", &mSeparation )
                         .minValue(0.0f)
                         .maxValue(2.0f));
    interface->addParam(CreateFloatParam( "Detail", &mDetail ));
    interface->addParam(CreateIntParam( "Strands", &mStrands )
                         .minValue(1)
                         .maxValue(50));
    interface->addParam(CreateFloatParam( "Scale", &mScale )
                         .minValue(0.0f)
                         .maxValue(10.0f));
    interface->addParam(CreateFloatParam( "Thickness", &mThickness )
                         .minValue(0.001f));
    interface->addParam(CreateBoolParam( "ColorSep", &mColorSeparation ));
    interface->addParam(CreateBoolParam( "BackgroundFlash", &mBackgroundFlash ));
    
    //    interface->gui()->addColumn();
    //    vector<string> bgReactionNames;
    //#define CIRCLEWAVE_BG_REACTION_ENTRY( nam, enm ) \
    //bgReactionNames.push_back(nam);
    //    CIRCLEWAVE_BG_REACTION_TUPLE
    //#undef  CIRCLEWAVE_BG_REACTION_ENTRY
    //    interface->addEnum(CreateEnumParam( "bg_reaction", (int*)(&mBackgroundReaction) )
    //                       .maxValue(BG_REACTION_COUNT)
    //                       .oscReceiver(name)
    //                       .isVertical(), bgReactionNames);
}

void CircleWave::Circle::update(double dt)
{
    
}

void CircleWave::Circle::setCustomParams( AudioInputHandler& audioInputHandler )
{
    mShader.uniform( "iSeparation", mSeparation );
    mShader.uniform( "iDetail", mDetail );
    mShader.uniform( "iStrands", mStrands );
    mShader.uniform( "iScale", mScale );
    mShader.uniform( "iColorSep", mColorSeparation );
    mShader.uniform( "iThickness", mThickness );
    mShader.uniform( "iBackgroundFlash", mBackgroundFlash );
    //mShader.unfirom( "iBgReaction", (int)mBackgroundReaction );
}

#pragma mark - SparkWave

CircleWave::Spark::Spark()
: FragShader("spark", "sparkwave.frag")
{
}

void CircleWave::Spark::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
}

void CircleWave::Spark::update(double dt)
{
    
}

void CircleWave::Spark::setCustomParams( AudioInputHandler& audioInputHandler )
{
    
}