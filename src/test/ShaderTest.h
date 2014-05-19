//
//  ShaderTest.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2011-10-27.
//
//

#pragma once

#include "AudioInputHandler.h"
#include "FragShader.h"
#include "GridRenderer.h"
#include "MotionBlurRenderer.h"
#include "OscMessage.h"
#include "Scene.h"
#include "SimplexNoiseTexture.h"
#include "TimeController.h"
#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include <vector>


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
    void shaderPostDraw();
    
private:

    bool                mMotionBlur;
    bool                mGrid;
    enum { MAX_TEXTURES = 4 };
    ci::gl::Texture         mTexture[MAX_TEXTURES];
    int                 mTextureIndex;
    
    MotionBlurRenderer  mMotionBlurRenderer;
    GridRenderer        mGridRenderer;
    
    std::vector<FragShader*> mShaders;
    int mShaderType;
    
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
