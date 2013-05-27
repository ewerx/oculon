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
#include "AudioInputHandler.h"

#include "OscMessage.h"

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
    //void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    bool handleKeyDown(const KeyEvent& keyEvent);
    //void handleMouseDown( const ci::app::MouseEvent& mouseEvent );
	void handleMouseDrag( const ci::app::MouseEvent& event );
    void handleOscMessage( const ci::osc::Message& message );
    
protected:// from Scene
    void setupInterface();
    //void setupDebugInterface();
    
private:
    void setupShaders();
    void updateBlur();
    void drawScene();
    
    void shaderPreDraw();
    void drawShaderOutput();
    void shaderPostDraw();
    
private:

    enum 
    { 
        FBO_COUNT = 4,
    };
    
    ci::gl::Fbo         mFbo[FBO_COUNT];
    int                 mFboIndex;
    
    ci::gl::GlslProg    mShader;
    
    Vec2f               mPos;
    Vec2f               mVel;
    ColorA              mColor;

    bool                mUseFbo;
    float               mBlurAmount;
    gl::Texture         mTexture;
    
    MotionBlurRenderer  mMotionBlurRenderer;
    
    std::vector<ci::gl::GlslProg> mShaders;
    
    // shaders
#define SHADERS_TUPLE \
SHADERS_ENTRY( "Simplicity", SHADER_SIMPLICITY ) \
SHADERS_ENTRY( "Menger", SHADER_MENGER ) \
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
    
    // audio
    AudioInputHandler   mAudioInputHandler;
    
    // SIMPLICITY
    ci::Vec3f           mColorScale;
    int                 mRedPower;
    int                 mGreenPower;
    int                 mBluePower;
    float               mStrengthFactor;
    float               mStrengthMin;
    float               mStrengthConst;
    int                 mIterations;
    float               mAccumPower;
    ci::Vec3f           mMagnitude;
    float               mFieldScale;
    float               mFieldSubtract;
    float               mTimeScale;
    ci::Vec3f           mPanSpeed;
    ci::Vec3f           mUVOffset;
    float               mUVScale;
    
};

#endif // __MOVIETEST_H__
