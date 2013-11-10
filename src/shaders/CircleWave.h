//
//  CircleWave.h
//  Oculon
//
//  Created by Ehsan on 13-10-27.
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
class CircleWave : public Scene
{
public:
    CircleWave();
    virtual ~CircleWave();
    
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
    //AudioInputHandler   mAudioInputHandler;
    
    // params
    ci::ColorAf         mColor1;
    ci::ColorAf         mColor2;
    float               mTimeScale;
    double              mElapsedTime;
    
    float               mSeparation;
    float               mDetail;
    int                 mStrands;
    float               mScale;
    bool                mColorSeparation;
    float               mThickness;
    
    // background
//#define CIRCLEWAVE_BG_REACTION_TUPLE \
//CIRCLEWAVE_BG_REACTION_ENTRY( "None", BG_REACTION_NONE ) \
//CIRCLEWAVE_BG_REACTION_ENTRY( "White", BG_REACTION_WHITE ) \
//CIRCLEWAVE_BG_REACTION_ENTRY( "Color", BG_REACTION_COLOR ) \
////end tuple
//    
//    enum eBackgroundReaction
//    {
//#define CIRCLEWAVE_BG_REACTION_ENTRY( nam, enm ) \
//enm,
//        CIRCLEWAVE_BG_REACTION_TUPLE
//#undef  CIRCLEWAVE_BG_REACTION_ENTRY
//        
//        BG_REACTION_COUNT
//    };
//    eBackgroundReaction mBackgroundReaction;
    
    bool                mBackgroundFlash;
};

