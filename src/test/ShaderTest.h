/*
 *  ShaderTest.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __SHADEREST_H__
#define __SHADEREST_H__

#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "Scene.h"
#include "MotionBlurRenderer.h"
#include "GridRenderer.h"
#include "AudioInputHandler.h"
#include "TimeController.h"
#include "OscMessage.h"
#include "SimplexNoiseTexture.h"

//
// Audio input tests
//
class ShaderTest : public Scene
{
public:
    ShaderTest();
    virtual ~ShaderTest();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    //bool handleKeyDown(const KeyEvent& keyEvent);
    //void handleMouseDown( const ci::app::MouseEvent& mouseEvent );
	//void handleMouseDrag( const ci::app::MouseEvent& event );
    //void handleOscMessage( const ci::osc::Message& message );
    
protected:// from Scene
    void setupInterface();
    ////void setupDebugInterface();
    
private:
    void setupShaders();
    void drawScene();
    
    void shaderPreDraw();
    void drawShaderOutput();
    void shaderPostDraw();
    
private:

    bool                mMotionBlur;
    bool                mGrid;
    enum { MAX_TEXTURES = 4 };
    ci::gl::Texture         mTexture[MAX_TEXTURES];
    int                 mTextureIndex;
    
    MotionBlurRenderer  mMotionBlurRenderer;
    GridRenderer        mGridRenderer;
    
    std::vector<ci::gl::GlslProg> mShaders;
    
    // shaders
#define SHADERS_TUPLE \
SHADERS_ENTRY( "Test",      "test_frag.glsl", SHADER_TEST ) \
SHADERS_ENTRY( "Fire2D",    "fire2d_frag.glsl", SHADER_FIRE2D ) \
SHADERS_ENTRY( "Cubes",    "cubes_frag.glsl", SHADER_CUBES ) \
SHADERS_ENTRY( "CloudLight",    "cloudlight_frag.glsl", SHADER_CUBES0 ) \
SHADERS_ENTRY( "GooSpin",    "goospin_frag.glsl", SHADER_GOOSPIN ) \
SHADERS_ENTRY( "Urchin",    "urchin_frag.glsl", SHADER_CUBES1 ) \
SHADERS_ENTRY( "FrameGrid",    "framegrid_frag.glsl", SHADER_CUBES2 ) \
SHADERS_ENTRY( "Sacred",    "sacred_frag.glsl", SHADER_CUBES3 ) \
SHADERS_ENTRY( "FluffyCloud",    "fluffycloud_frag.glsl", SHADER_CUBES4 ) \
SHADERS_ENTRY( "Triangle",    "triangle_frag.glsl", SHADER_CUBES5 ) \
SHADERS_ENTRY( "Tripping",  "tripping_frag.glsl", SHADER_TRIPPING ) \
SHADERS_ENTRY( "Stripes",   "stripes_frag.glsl", SHADER_STRIPES ) \
SHADERS_ENTRY( "Flicker",   "energyflicker_frag.glsl", SHADER_FLICKER ) \
SHADERS_ENTRY( "Inversion", "inversion_frag.glsl", SHADER_INVERSION ) \
SHADERS_ENTRY( "Cymatics",  "cymatics_frag.glsl", SHADER_CYMATICS ) \
SHADERS_ENTRY( "Logistic",          "logistic_frag.glsl", SHADER_LOGISTIC ) \
SHADERS_ENTRY( "Interstellar",      "interstellar_frag.glsl", SHADER_INTERSTELLAR ) \
SHADERS_ENTRY( "MainSequence",      "mainsequence_frag.glsl", SHADER_MAINSEQUENCE ) \
SHADERS_ENTRY( "InfiniteFall",      "infinitefall_frag.glsl", SHADER_INFINITEFALL ) \
//SHADERS_ENTRY( "AfterEffect",       "aftereffect_frag.glsl", SHADER_AFTEREFFECT ) \
//SHADERS_ENTRY( "Rasterizer", SHADER_RASTERIZER ) \
//SHADERS_ENTRY( "Glassfield", SHADER_GLASSFIELD ) \
//SHADERS_ENTRY( "Cosmos", SHADER_COSMOS ) \
//SHADERS_ENTRY( "Glow", SHADER_LIGHTGLOW ) \
//SHADERS_ENTRY( "Cloud", SHADER_CLOUD ) \
//SHADERS_ENTRY( "MetaHexBalls", SHADER_METAHEXBALLS ) \
//SHADERS_ENTRY( "Stripey", SHADER_STRIPEY ) \
//SHADERS_ENTRY( "Moire", SHADER_MOIRE ) \
//SHADERS_ENTRY( "Polychora", SHADER_POLYCHORA ) \
//end tuple
    
    enum eShaderType
    {
#define SHADERS_ENTRY( nam, glsl, enm ) \
enm,
        SHADERS_TUPLE
#undef  SHADERS_ENTRY
        
        SHADERS_COUNT
    };
    eShaderType   mShaderType;
    
    //TEST
    int                 mRadius;
    
    struct tInversionParams
    {
        float mWidth;
        float mScale;
        float mDetail;
        ci::Vec3f mLightDir;
        float mTimeScale;
        float mShakeSpeed;
        float mShakeAmount;
        float mLightPow;
        float mLightScale;
        float mLightRange;
        float mContrast;
    };
    tInversionParams mInversionParams;
    
    struct tStripesParams
    {
        float mTimeScale;
        float mCountScale;
        float mCountScaleMult;
        ci::ColorAf mColor1;
        ci::ColorAf mColor2;
        
    };
    tStripesParams mStripesParams;
    
    struct tAfterEffectsParams
    {
        int mEffect;
    };
    tAfterEffectsParams mAfterEffectParams;
    
    TimeController mTimeController;
    
    SimplexNoiseTexture mDynamicTexture;
};

#endif // __MOVIETEST_H__
