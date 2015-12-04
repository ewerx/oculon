//
//  Tilings.h
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

#pragma once

#include "cinder/Cinder.h"
#include "TextureShaders.h"
#include "FragShader.h"
#include "AudioBandSelector.h"

//
// 2D Line Patterns
//
class Tilings : public TextureShaders
{
public:
    Tilings(const std::string& name);
    virtual ~Tilings();
    
    
private:
    void setupShaders() override;
    
private:
    
#pragma mark -
    class Tessellations: public FragShader
    {
    public:
        Tessellations();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
//        int                 mOffsetResponse;
//        int                 mAnglePResponse;
//        int                 mAngleQResponse;
//        int                 mAngleRResponse;
        
        int                 mIterations;
//        int                 mAngleP;
//        int                 mAngleQ;
//        int                 mAngleR;
        ci::Vec3f           mCenter;
        float               mThickness;
//        float               mOffset;
        float               mHOffset;
        float               mScale;
        
        AudioFloatParam mAngleP;
        AudioFloatParam mAngleQ;
        AudioFloatParam mAngleR;
        AudioFloatParam mOffset;
    };
    
#pragma mark -
    class Voronoi: public FragShader
    {
    public:
        Voronoi();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        int                 mResponseBand;
        bool                mAudioOffset;
        
        ci::Vec3f mBorderColor;
        float mSpeed;
        float mZoom;
        float mBorderIn;
        float mBorderOut;
        ci::Vec3f mSeedColor;
        float mSeedSize;
        float mCellLayers;
        ci::Vec3f mCellColor;
        float mCellBorderStrength;
        float mCellBrightness;
        float mDistortion;
        bool mAudioDistortion;
    };
    
#pragma mark - VoronoiPlasma
    
    class VoronoiPlasma : public FragShader
    {
    public:
        VoronoiPlasma();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        
    };
    
#pragma mark - VoronoiFire
    
    class VoronoiFire : public FragShader
    {
    public:
        VoronoiFire();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        int mLayers;
        //float mAmount;
        AudioFloatParam mAmount;
        float mTurbulence;
        //float mAmpScale;
        AudioFloatParam mAmpScale;
        ci::Vec2f mShiftRate;
        float mSpinRate;
        //float mScale;
        AudioFloatParam mScale;
        float mNoise;
        //float mContrast;
        AudioFloatParam mContrast;
    };
    
#pragma mark - VoronoiCells
    
    class VoronoiCells : public FragShader
    {
    public:
        VoronoiCells();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        float mGap;
        float mAgitation;
        float mZoom;
    };
};
