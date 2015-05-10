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

#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"

class EffectShaders : public Scene
{
public:
    EffectShaders();
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
    TextureSelector mInputTextures;
    
public:
#pragma mark -
    
    class CathodeRay : public FragShader
    {
    public:
        CathodeRay();
        void setupInterface( Interface* interface, const std::string& prefix );
        void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        float   mPowerBandThickness;
        int     mPowerBandThicknessResponse;
        float   mPowerBandIntensity;
        int     mPowerBandIntensityResponse;
        float   mPowerBandSpeed;
        int     mPowerBandSpeedResponse;
        float   mSignalNoise;
        int     mSignalNoiseResponse;
        float   mScanlines;
        
        ci::ColorAf  mTintColor;
        
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
    class Television : public FragShader
    {
    public:
        Television();
        void setupInterface( Interface* interface, const std::string& prefix );
        void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        
    };
};




