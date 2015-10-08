//
//  TextureShaders.h
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

#pragma once

#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "Scene.h"
#include "MotionBlurRenderer.h"
#include "GridRenderer.h"
#include "AudioInputHandler.h"
#include "TimeController.h"
#include "FragShader.h"
#include "TextureSelector.h"

//
// TextureShaders
//
class TextureShaders : public Scene
{
public:
    TextureShaders(const std::string& name);
    virtual ~TextureShaders();
    
    // inherited from Scene
    void setup();
    void reset();
    virtual void update(double dt);
    void draw();
    //void drawDebug();
    
    void setInputTexture( ci::gl::TextureRef texture ) { mInputTexture = texture; }
    
protected:// from Scene
    void setupInterface();
    //void setupDebugInterface();
    
protected:
    virtual void setupShaders();
    
    void drawScene();
    
    void shaderPreDraw();
    void shaderPostDraw();
    
protected:
    
    // global params
    ci::ColorAf         mColor1;
    ci::ColorAf         mColor2;
    
    TimeController      mTimeController;
    AudioInputHandler   mAudioInputHandler;
    
    // WTF, if mInputTexture is decalered here there are runtime bad access errors?!
    
    // color maps
    TextureSelector     mColorMaps;
    
    // shaders
    std::vector<FragShader*> mShaders;
    int mShaderType;
    

    ci::gl::TextureRef  mInputTexture;
    
private:
    class KifsShader : public FragShader
    {
    public:
        KifsShader();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        int iterations;
        float scale;
        ci::Vec2f fold;
        ci::Vec2f translate;
        float zoom;
        float brightness;
        float saturation;
        float texturescale;
        
        float rotspeed;
        float mRotation;
        float mRotationOffset;
        
        float colspeed;
        float mColorOffset;
        
        float antialias;
        
        bool mAudioFold;
        bool mAudioTranslate;
        bool mAudioRot;
    };
    
    class SimplicityShader : public FragShader
    {
    public:
        SimplicityShader();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        
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
        ci::Anim<ci::Vec3f> mPanPos;
        ci::Vec3f           mUVOffset;
        ci::Anim<float>     mUVScale;
        
        bool                mAudioHighlight;
        bool                mAudioShift;
    };
    
    class BezierShader : public FragShader
    {
    public:
        BezierShader();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        float   mThickness;
        float   mBlur;
        float   mFrequency;
        int     mSamples;
        float   mPoint1Range;
        float   mPoint2Range;
        
        int     mBlurResponseBand;
    };
    
    class PixelWeaveShader : public FragShader
    {
    public:
        PixelWeaveShader();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        //virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        int mBokeh;
        int mSpacing;
        
    };
    
    class InfiniteFall : public FragShader
    {
    public:
        InfiniteFall();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        //virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        
    };

    class InfiniteVoronoi : public FragShader
    {
    public:
        InfiniteVoronoi();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        //virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        
    };
};

