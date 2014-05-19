//
//  Menger.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-17.
//
//

#pragma once

#include "Scene.h"
#include "TimeController.h"
#include "AudioInputHandler.h"
#include "FragShader.h"

#include "cinder/gl/GlslProg.h"

class Menger : public Scene
{
public:
    Menger();
    virtual ~Menger();
    
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
    std::vector<FragShader*> mShaders;
    int mShaderIndex;
    
    // params
    ci::ColorAf         mColor1;
    ci::ColorAf         mColor2;
    float               mAmbientLight;
    float               mDiffuseLight;
    ci::Vec3f           mLight1Dir;
    ci::Vec3f           mLight2Dir;
    
    class MengerShader : public FragShader
    {
    public:
        MengerShader();
        void setupInterface( Interface* interface, const std::string& name );
        void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        int                 mIterations;
        int                 mMaxSteps;
        float               mFieldOfView;
        float               mScale;
        float               mJitter;
        float               mFudgeFactor;
        float               mPerspective;
        float               mMinDistance;
        float               mNormalDistance;
        ci::Vec3f           mOffset;
        int                 mOffsetXResponseBand;
        int                 mOffsetYResponseBand;
        int                 mOffsetZResponseBand;
    };
    
    class PolychoraShader : public FragShader
    {
    public:
        PolychoraShader();
        void setupInterface( Interface* interface, const std::string& name );
        void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        int                 mZone;
        int                 mMaxSteps;
        float               mFieldOfView;
        float               mJitter;
        float               mFudgeFactor;
        float               mAngle;
        int                 mAngleResponseBand;
        float               mMinDistance;
        float               mNormalDistance;
    };
    
    // control
    TimeController      mTimeController;
    AudioInputHandler   mAudioInputHandler;
    int                 mResponseBand;
};