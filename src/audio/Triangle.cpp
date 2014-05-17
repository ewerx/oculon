/*
 *  Triangle.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */


#include "Triangle.h"
#include "Utils.h"
#include "Scene.h"
#include "OculonApp.h"
#include "Interface.h"
#include "Constants.h"
#include "Resources.h"

#include "cinder/gl/gl.h"
#include "cinder/CinderMath.h"
#include "cinder/Rand.h"
#include "cinder/Easing.h"
#include "cinder/ObjLoader.h"

using namespace ci;
using namespace std;


//
// Triangle
// 

Triangle::Triangle(Scene* scene)
: SubScene(scene,"Triangle")
{
}

Triangle::~Triangle()
{
}

void Triangle::setup()
{
    // params
    
    
    reset();
}

void Triangle::setupInterface()
{
    SubScene::setupInterface();
    
    Interface* interface = mParentScene->getInterface();
    
//    interface->addParam(CreateFloatParam("Scale", &mBaseRadius)
//                        .minValue(1.0f)
//                        .maxValue(200.0f)
//                        .oscReceiver(mName,"scale"));//->registerCallback( this, &Triangle::baseRadiusChanged );
//
//    interface->addParam(CreateFloatParam("left", &mLeft.x)
//                        .minValue(-mApp->getWindowWidth())
//                        .maxValue(mApp->getWindowWidth()));
//    interface->addParam(CreateFloatParam("right", &mRight.x)
//                        .minValue(-mApp->getWindowWidth())
//                        .maxValue(mApp->getWindowWidth()));
//    interface->addParam(CreateFloatParam("topx", &mTop.x)
//                        .minValue(-100.f)
//                        .maxValue(500.f);
//    interface->addParam(CreateBoolParam("Alt Shift X", &mShiftAlternateX));
//    interface->addParam(CreateBoolParam("Alt Shift Y", &mShiftAlternateY));
    
}

//void Triangle::setupDebugInterface()
//{
//}

void Triangle::reset()
{
    const float windowHeight = mParentScene->getApp()->getViewportHeight();
    const float windowWidth = mParentScene->getApp()->getViewportWidth();
    

}

void Triangle::resize()
{
}

void Triangle::update(double dt)
{
}

void Triangle::draw()
{
    // audio data
    AudioInput& audioInput = mParentScene->getApp()->getAudioInput();
    //float * freqData = audioInput.getFft()->getAmplitude();
    //float * timeData = audioInput.getFft()->getData();
//    int32_t dataSize = audioInput.getFft()->getBinSize();
//    const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
    // dimensions
    const float windowHeight = mParentScene->getApp()->getViewportHeight();
    const float windowWidth = mParentScene->getApp()->getViewportWidth();
    
    gl::pushMatrices();
    
        
    //console() << "--------------------------\n";
    
    gl::popMatrices();

}