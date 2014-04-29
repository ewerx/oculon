//
//  FractalShaders.h
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
class FractalShaders : public Scene
{
public:
    FractalShaders();
    virtual ~FractalShaders();
    
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
    
private:
    
    // global params
    ci::ColorAf         mColor1;
    ci::ColorAf         mColor2;
    float               mTimeScale;
    double              mElapsedTime;
    
    // shaders
    std::vector<ci::gl::GlslProg> mShaders;
#define SHADERS_TUPLE \
SHADERS_ENTRY( "Simplicity", SHADER_SIMPLICITY ) \
SHADERS_ENTRY( "KIFS", SHADER_PAINT ) \
SHADERS_ENTRY( "Tripping", SHADER_TRIPPING ) \
SHADERS_ENTRY( "Tilings", SHADER_TILINGS ) \
SHADERS_ENTRY( "Flicker", SHADER_FLICKER ) \
SHADERS_ENTRY( "Inversion", SHADER_INVERSION ) \
SHADERS_ENTRY( "Cymatics", SHADER_CYMATICS ) \
SHADERS_ENTRY( "Logistic", SHADER_LOGISTIC ) \
SHADERS_ENTRY( "Interstellar", SHADER_INTERSTELLAR ) \
SHADERS_ENTRY( "Stripey", SHADER_STRIPEY ) \
SHADERS_ENTRY( "Moire", SHADER_MOIRE ) \
SHADERS_ENTRY( "VolumetricLines", SHADER_VOLUMETRICLINES ) \
SHADERS_ENTRY( "SquareNoise", SHADER_SQUARENOISE ) \
SHADERS_ENTRY( "AfterEffect", SHADER_AFTEREFFECT ) \
SHADERS_ENTRY( "MainSequence", SHADER_MAINSEQUENCE ) \
//SHADERS_ENTRY( "Rasterizer", SHADER_RASTERIZER ) \
//SHADERS_ENTRY( "Glassfield", SHADER_GLASSFIELD ) \
//SHADERS_ENTRY( "Cosmos", SHADER_COSMOS ) \
//SHADERS_ENTRY( "Glow", SHADER_LIGHTGLOW ) \
//SHADERS_ENTRY( "Cloud", SHADER_CLOUD ) \
//SHADERS_ENTRY( "MetaHexBalls", SHADER_METAHEXBALLS ) \
//SHADERS_ENTRY( "Menger", SHADER_MENGER ) \
//SHADERS_ENTRY( "Polychora", SHADER_POLYCHORA ) \
//end tuple
    
    enum eShaderType
    {
#define SHADERS_ENTRY( nam, enm ) \
enm,
        SHADERS_TUPLE
#undef  SHADERS_ENTRY
        
        SHADERS_COUNT
    };
    eShaderType   mShaderType;
    
    // shader params
    struct tKaliParams
    {
        int iterations;
        float scale;
        ci::Vec2f fold;
        ci::Vec2f translate;
        float zoom;
        float brightness;
        float saturation;
        float texturescale;
        
        float rotspeed;
        
        float colspeed;
        
        float antialias;
    };
    tKaliParams mKaliParams;
};

