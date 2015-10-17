//
//  EffectShaders.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-15.
//
//

#pragma once

#include "Scene.h"
#include "TimeController.h"
#include "AudioInputHandler.h"
#include "SimplexNoiseTexture.h"
#include "FragShader.h"
#include "TextureSelector.h"
#include "AudioBandSelector.h"

#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"

class EffectShaders : public Scene
{
public:
    EffectShaders(const std::string& name = "effects");
    virtual ~EffectShaders();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    
protected:// from Scene
    void setupInterface();
    
private:
    void shaderPreDraw();
    void shaderPostDraw();
    
private:
    
    // control
    TimeController      mTimeController;
    AudioInputHandler   mAudioInputHandler;
    
    // noise
    SimplexNoiseTexture mDynamicNoiseTexture;
    
    TextureSelector mNoiseTextures;
    
    // effects
    std::vector<FragShader*> mEffects;
    int mCurrentEffect;
    
    // inputs
    TextureSelector mInput1Texture;
    TextureSelector mInput2Texture;
    TimelineFloatParam mInput1Alpha;
    TimelineFloatParam mInput2Alpha;
    
public:
#pragma mark -
    
    class CathodeRay : public FragShader
    {
    public:
        CathodeRay();
        void setupInterface( Interface* interface, const std::string& prefix );
        void setCustomParams( AudioInputHandler& audioInputHandler );
        void update(double dt);
        
    private:
        float   mPowerBandThickness;
        int     mPowerBandThicknessResponse;
        float   mPowerBandIntensity;
        int     mPowerBandIntensityResponse;
        TimeController mPowerBandTime;
        float   mSignalNoise;
        int     mSignalNoiseResponse;
        float   mScanlines;
        int     mScanlinesBand;
        float   mColorShift;
        AudioBandSelector     mColorShiftBand;
//        float   mInputAlpha;
        AudioFloatParam mFrameShift;
        AudioFloatParam mScanShift;
        
        ci::ColorAf  mTintColor;
        
    };
    
#pragma mark -
    class Television : public FragShader
    {
    public:
        Television();
        void setupInterface( Interface* interface, const std::string& prefix );
        void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        float mVerticalJerk;
        float mVerticalShift;
        float mBottomStatic;
        float mScanlines;
        float mColorShift;
        float mHorizontalFuzz;
    };
    
#pragma mark -
    class VideoTape : public FragShader
    {
    public:
        VideoTape();
        void setupInterface( Interface* interface, const std::string& prefix );
        void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        
    };

#pragma mark -
    class OilWater : public FragShader
    {
    public:
        OilWater();
        void setupInterface( Interface* interface, const std::string& prefix );
        void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        
    };

};


