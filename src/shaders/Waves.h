//
//  Waves.h
//  Oculon1
//
//  Created by Ehsan Rezaie on 2015-05-05.
//
//

#pragma once

#include "cinder/Cinder.h"
#include "TextureShaders.h"
#include "FragShader.h"

//
// Waves
//
class Waves : public TextureShaders
{
public:
    Waves();
    virtual ~Waves();
    
private:
    void setupShaders() override;
    
private:
    
    class MultiWave : public FragShader
    {
    public:
        MultiWave();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        int mNumBands;
        float mGlowWidth;
        float mGlowLength;
        float mIntensity;
        float mWaveRate;
        float mCurvature;
        float mSeparation;
    };
    
    class SineWave : public FragShader
    {
    public:
        SineWave();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        
    };
    
    class Oscilloscope : public FragShader
    {
    public:
        Oscilloscope();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        
    };
    
    class AudioGraph : public FragShader
    {
    public:
        AudioGraph();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
//        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        float mSmoothness;
        float mLength;
    };
    
    class Oscillator : public FragShader
    {
    public:
        Oscillator();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        struct tWaveParams
        {
            ci::Anim<ci::ColorAf>   mColor;
            float                   mElapsedTime;
            float                   mTimeScale;
            float                   mOffset;
            ci::Anim<float>         mAmplitude;
            ci::Anim<float>         mFrequency;
            bool                    mAudioPhase;
            bool                    mAudioAmp;
        };
        enum { MAX_WAVES = 1 };
        tWaveParams mWaveParams[MAX_WAVES];
        
        float         mTimeScale;
        float         mAmplitude;
        float         mFrequency;
        bool          mAudioPhase;
        bool          mAudioAmp;
    };
};
