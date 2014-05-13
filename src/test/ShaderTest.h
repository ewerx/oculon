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
SHADERS_ENTRY( "Test", SHADER_TEST ) \
SHADERS_ENTRY( "Tripping", SHADER_TRIPPING ) \
SHADERS_ENTRY( "Stripes", SHADER_STRIPES ) \
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
SHADERS_ENTRY( "InfiniteFall", SHADER_INFINITEFALL ) \
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
