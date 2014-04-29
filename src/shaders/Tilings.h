//
//  Tilings.h
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

#pragma once

#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "Scene.h"
#include "MotionBlurRenderer.h"
#include "GridRenderer.h"
#include "AudioInputHandler.h"

#include "OscMessage.h"

//
// Audio input tests
//
class Tilings : public Scene
{
public:
    Tilings();
    virtual ~Tilings();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    
protected:// from Scene
    void setupInterface();
    ////void setupDebugInterface();
    
private:
    void drawScene();
    
    void shaderPreDraw();
    void drawShaderOutput();
    void shaderPostDraw();
    
    float getAudioLevelForBand(int bandIndex);
    
private:

    ci::gl::GlslProg    mShader;
    
    // audio
    AudioInputHandler   mAudioInputHandler;
    int                 mOffsetResponse;
    int                 mAnglePResponse;
    int                 mAngleQResponse;
    int                 mAngleRResponse;
    
    // params
    ci::ColorAf         mColor1;
    ci::ColorAf         mColor2;
    float               mTimeScale;
    double              mElapsedTime;
    
    int                 mIterations;
    int                 mAngleP;
    int                 mAngleQ;
    int                 mAngleR;
    ci::Vec3f           mCenter;
    float               mThickness;
    float               mOffset;
    float               mHOffset;
    float               mScale;
};

