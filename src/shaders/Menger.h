//
//  Menger.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-17.
//
//

#pragma once

#include "Scene.h"
#include "TimeController.h"
#include "AudioInputHandler.h"

#include "cinder/gl/GlslProg.h"

class Menger : public Scene
{
public:
    Menger();
    virtual ~Menger();
    
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
    
    int                 mIterations;
    int                 mMaxSteps;
    float               mFieldOfView;
    float               mScale;
    float               mJitter;
    float               mFudgeFactor;
    float               mPerspective;
    float               mMinDistance;
    float               mNormalDistance;
    
    float               mAmbientLight;
    float               mDiffuseLight;
    ci::Vec3f           mLight1Dir;
    ci::Vec3f           mLight2Dir;
    ci::Vec3f           mOffset;
    
    // control
    TimeController      mTimeController;
    AudioInputHandler   mAudioInputHandler;
    int                 mResponseBand;
};