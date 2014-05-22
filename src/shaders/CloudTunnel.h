//
//  CloudTunnel.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-22.
//
//

#pragma once

#include "Scene.h"
#include "TimeController.h"
#include "AudioInputHandler.h"
#include "FragShader.h"

#include "cinder/gl/GlslProg.h"

class CloudTunnel : public Scene
{
public:
    CloudTunnel();
    virtual ~CloudTunnel();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    
protected:// from Scene
    void setupInterface();
    
private:
    void shaderPreDraw();
    void shaderPostDraw();
    
private:
    ci::gl::GlslProg    mShader;
    
    // params
    ci::ColorAf         mColor1;
    ci::ColorAf         mColor2;
    
    int                 mIterations;
    float               mDensity;
    int                 mDensityResponseBand;
    float               mFieldOfView;
    int                 mFOVResponseBand;
    float               mNoiseLevel;
    int                 mNoiseLevelResponseBand;
    float               mTunnelWidth;
    int                 mTunnelWidthResponseBand;
    
    // control
    TimeController      mTimeController;
    AudioInputHandler   mAudioInputHandler;
    int                 mResponseBand;
};