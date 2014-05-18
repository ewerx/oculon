//
//  Cells.h
//  Oculon
//
//  Created by Ehsan on 13-11-15.
//  Copyright 2013 ewerx. All rights reserved.
//

#pragma once

#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "Scene.h"
#include "MotionBlurRenderer.h"
#include "GridRenderer.h"
#include "AudioInputHandler.h"

#include "OscMessage.h"

//
// Audio input tests
//
class Cells : public Scene
{
public:
    Cells();
    virtual ~Cells();
    
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
    void shaderPostDraw();
    
private:

    ci::gl::GlslProg    mShader;
    
    AudioInputHandler mAudioInputHandler;
    
    float               mAudioResponseFreqMin;
    float               mAudioResponseFreqMax;
    
#define AUDIO_RESPONSE_TYPE_TUPLE \
AUDIO_RESPONSE_TYPE_ENTRY( "None", AUDIO_RESPONSE_NONE ) \
AUDIO_RESPONSE_TYPE_ENTRY( "SingleBand", AUDIO_RESPONSE_SINGLE ) \
AUDIO_RESPONSE_TYPE_ENTRY( "MultiBand", AUDIO_RESPONSE_MULTI ) \
//end tuple
    
    enum eAudioRessponseType
    {
#define AUDIO_RESPONSE_TYPE_ENTRY( nam, enm ) \
enm,
        AUDIO_RESPONSE_TYPE_TUPLE
#undef  AUDIO_RESPONSE_TYPE_ENTRY
        
        AUDIO_RESPONSE_TYPE_COUNT
    };
    eAudioRessponseType mAudioResponseType;
    
    ci::ColorAf         mColor1;
    ci::ColorAf         mColor2;
    
    float mZoom;
    float mHighlight;
    
    enum { CELLS_NUM_LAYERS = 7 };
    
    float mTimeStep[CELLS_NUM_LAYERS];
    float mFrequency[CELLS_NUM_LAYERS];
    float mTime[CELLS_NUM_LAYERS];
    
    float mIntensity;

};