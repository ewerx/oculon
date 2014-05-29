//
//  Cymatics.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-18.
//
//

#pragma once

#include "Scene.h"
#include "TimeController.h"
#include "AudioInputHandler.h"

#include "cinder/gl/GlslProg.h"

class Cymatics : public Scene
{
public:
    Cymatics();
    virtual ~Cymatics();
    
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
    
    int                 mPoles;
    float               mDistance;
    float               mShift;
    float               mBlur;
    int                 mDistanceResponseBand;
    int                 mPolesResponseBand;
    
    // control
    TimeController      mTimeController;
    AudioInputHandler   mAudioInputHandler;
    int                 mResponseBand;
};