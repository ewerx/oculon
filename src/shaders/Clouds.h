//
//  Clouds.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-22.
//
//

#pragma once

#include "cinder/Cinder.h"
#include "TextureShaders.h"
#include "FragShader.h"
#include "SimplexNoiseTexture.h"
#include "TextureSelector.h"

//
// Clouds
//
class Clouds : public TextureShaders
{
public:
    Clouds();
    virtual ~Clouds();
    
    void setupInterface() override;
    void update(double dt) override;
    
private:
    void setupShaders() override;
    
private:
    // noise
    TextureSelector mNoiseTextures;
    ci::gl::Texture mStaticNoiseTexture;
    SimplexNoiseTexture mDynamicNoiseTexture;
    
    
#pragma mark-
    class CloudTunnel : public FragShader
    {
    public:
        CloudTunnel();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        int                 mIterations;
        float               mDensity;
        int                 mDensityResponseBand;
        float               mFieldOfView;
        int                 mFOVResponseBand;
        float               mNoiseLevel;
        int                 mNoiseLevelResponseBand;
        float               mTunnelWidth;
        int                 mTunnelWidthResponseBand;
        
        int                 mResponseBand;
    };
    
#pragma mark -
    class PlasmaFog : public FragShader
    {
    public:
        PlasmaFog();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        
    };
};