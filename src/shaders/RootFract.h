//
//  RootFract.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-13.
//
//

#pragma once

#include "Scene.h"
#include "TimeController.h"
#include "AudioInputHandler.h"

#include "cinder/gl/GlslProg.h"

class RootFract : public Scene
{
public:
    RootFract();
    virtual ~RootFract();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    
protected:// from Scene
    void setupInterface();
    
private:
    void shaderPreDraw();
    void drawShaderOutput();
    void shaderPostDraw();
    
private:
    ci::gl::GlslProg    mShader;
    
    // params
    ci::ColorAf         mColor1;
    ci::ColorAf         mColor2;
    ci::ColorAf         mColor3;
  
    int                 mIterations;
    float               mScale;
    float               mSaturation;
    float               mBrightness;
    float               mContrast;
    float               mMinBrightness;
    ci::Vec2f           mJulia;
    ci::Vec3f           mOrbitTraps;
    ci::Vec3f           mTrapWidths;
    ci::Vec3f           mTrapBrightness;
    ci::Vec3f           mTrapContrast;
    
    
    // control
    TimeController      mTimeController;
    AudioInputHandler   mAudioInputHandler;
    int                 mResponseBand;
};