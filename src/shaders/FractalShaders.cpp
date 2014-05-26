//
//  FractalShaders.cpp
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

#include "OculonApp.h"
#include "FractalShaders.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include <boost/format.hpp>
#include "Interface.h"
#include "Utils.h"

using namespace ci;

FractalShaders::FractalShaders()
: Scene("FractalShaders")
{
}

FractalShaders::~FractalShaders()
{
}

void FractalShaders::setup()
{
    Scene::setup();
    
    mShader = loadFragShader("FractalShaders_frag.glsl");
    
    reset();
}

void FractalShaders::reset()
{
    mElapsedTime = 0.0f;
}

void FractalShaders::setupInterface()
{
    mInterface->addParam(CreateFloatParam( "TimeScale", &mTimeScale )
                         .minValue(0.0f)
                         .maxValue(2.0f));
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor));
}

void FractalShaders::update(double dt)
{
    Scene::update(dt);
    
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    mElapsedTime += dt;
}

void FractalShaders::draw()
{
    gl::pushMatrices();

    drawScene();
    
    gl::popMatrices();
}

void FractalShaders::shaderPreDraw()
{
    mShader.bind();
    
    Vec2f resolution = Vec2f( mApp->getViewportWidth(), mApp->getViewportHeight() );
    
    mShader.uniform( "iResolution", resolution );
    mShader.uniform( "iGlobalTime", (float)mApp->getElapsedSeconds() );
}

void FractalShaders::shaderPostDraw()
{
    mShader.unbind();
}

void FractalShaders::drawScene()
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

void FractalShaders::drawDebug()
{
    mAudioInputHandler.drawDebug(mApp->getViewportSize());
}
