//
//  Voronoi.h
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
// Voronoi
//
class Voronoi : public Scene
{
public:
    Voronoi();
    virtual ~Voronoi();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    
protected:// from Scene
    void setupInterface();
    //void setupDebugInterface();
    
private:
    void drawScene();
    
    void shaderPreDraw();
    void drawShaderOutput();
    void shaderPostDraw();
    
private:

    ci::gl::GlslProg    mShader;
    
    // audio
    AudioInputHandler   mAudioInputHandler;
    int                 mResponseBand;
    bool                mAudioOffset;
    
    // params
    ci::ColorAf         mColor1;
    ci::ColorAf         mColor2;
    float               mTimeScale;
    double              mElapsedTime;
    
    ci::Vec3f mBorderColor;
    float mSpeed;
    float mZoom;
    float mBorderIn;
    float mBorderOut;
    ci::Vec3f mSeedColor;
    float mSeedSize;
    float mCellLayers;
    ci::Vec3f mCellColor;
    float mCellBorderStrength;
    float mCellBrightness;
    float mDistortion;
    bool mAudioDistortion;
};

