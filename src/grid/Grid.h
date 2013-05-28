//
// Grid.h
// Oculon
//
// Created by Ehsan on 12-12-28.
// Copyright 2011 ewerx. All rights reserved.
//


#pragma once


#include "AudioInputHandler.h"
#include "Scene.h"
#include "MotionBlurRenderer.h"
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Surface.h"
#include "cinder/Timeline.h"
#include <deque>


// CONSTANTS
#define GRID_WIDTH 35
#define GRID_HEIGHT 9

//
// Grid
//
class Grid : public Scene
{
public:
    Grid();
    virtual ~Grid();
    
    // inherited from Scene
    void setup();
    void reset();
    //void resize();
    void update(double dt);
    void draw();
    void drawDebug();
    
    //HACKS!
    ci::gl::Fbo& getVtfFbo() { return mVtfFbo; }
    
protected:// from Scene
    void setupInterface();
    void setupDebugInterface();
    
private:
    void updateAudioResponse();
    
    void setupDynamicTexture();
    void shaderPreDraw();
    void shaderPostDraw();
    void drawDynamicTexture();
    void drawFromDynamicTexture();
    void drawPixels();
    
    bool setColorScheme();
    
private:
    ci::gl::Texture             mTexture;
    
    // params
#define SHADER_TUPLE \
SHADER_ENTRY( "Noise", SHADER_NOISE ) \
SHADER_ENTRY( "Tunnel", SHADER_TUNNEL ) \
SHADER_ENTRY( "Fractal", SHADER_FRACTAL ) \
//end tuple
    
    enum eShaderType
    {
#define SHADER_ENTRY( nam, enm ) \
enm,
        SHADER_TUPLE
#undef  SHADER_ENTRY
        
        SHADER_COUNT
    };
    eShaderType                 mShaderType;
    ci::gl::GlslProg			mShaders[SHADER_COUNT];
    
    // PERLIN
    ci::gl::Fbo					mVtfFbo;
	ci::gl::GlslProg			mShaderTex;
	float						mDisplacementSpeed;
    float                       mDisplacementHeight;
	float						mTheta;
    ci::Vec3f                   mNoiseScale;
    
    // TUNNEL
    
    
    // pixelate
    ci::gl::GlslProg			mShaderPixelate;
    
    // motion blur
    MotionBlurRenderer          mMotionBlurRenderer;
    bool                        mMotionBlurEnabled;
    
    // pixel control
    struct tPixel
    {
        float mValue;
    };
    tPixel                      mPixels[GRID_WIDTH][GRID_HEIGHT];
    
    // audio response
    AudioInputHandler           mAudioInputHandler;
    
    // params
#define GRIDMODE_TUPLE \
GRIDMODE_ENTRY( "Shader", GRIDMODE_SHADER ) \
GRIDMODE_ENTRY( "Pixels", GRIDMODE_PIXELS ) \
//end tuple
    
    enum eGridMode
    {
#define GRIDMODE_ENTRY( nam, enm ) \
enm,
        GRIDMODE_TUPLE
#undef  GRIDMODE_ENTRY
        
        GRIDMODE_COUNT
    };
    eGridMode   mGridMode;
    
    // colors
#define COLORSCHEME_TUPLE \
COLORSCHEME_ENTRY( "RedFire", COLORSCHEME_REDFIRE ) \
COLORSCHEME_ENTRY( "BlueFire", COLORSCHEME_BLUEFIRE ) \
COLORSCHEME_ENTRY( "Ice", COLORSCHEME_ICE ) \
COLORSCHEME_ENTRY( "Green", COLORSCHEME_GREEN ) \
//end tuple
    
    enum eColorScheme
    {
#define COLORSCHEME_ENTRY( nam, enm ) \
enm,
        COLORSCHEME_TUPLE
#undef  COLORSCHEME_ENTRY
        
        COLORSCHEME_COUNT
    };
    eColorScheme    mColorScheme;
    
    ci::ColorAf     mColor1;
    ci::ColorAf     mColor2;
    ci::ColorAf     mColor3;

    float           mLowPassSplit;
    float           mHighPassSplit;
    
    int             mGroupCols;
    int             mGroupRows;
};

