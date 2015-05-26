//
//  SimplexNoiseTexture.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-04-29.
//
//

#pragma once

#include "TextureGenerator.h"
#include "Interface.h"
#include "TimeController.h"

//
// Simple Noise Dynamic Texture
//
class SimplexNoiseTexture : public TextureGenerator
{
public:
    SimplexNoiseTexture();
    virtual ~SimplexNoiseTexture();
    
    // TextureGenerator
    void initShader();
    void configShader(double dt);
    void setupInterface( Interface* interface, const std::string& name );
    
public:
    float       mNoiseSpeed;
	float       mNoiseTheta;
    ci::Vec3f   mNoiseScale;
    ci::Vec3f   mNoiseScaleMultiplier;
};

//
// Infinite Fall Noise
//
class InfiniteFallTexture : public TextureGenerator
{
public:
    InfiniteFallTexture() {}
    
    // TextureGenerator
    void initShader();
    void configShader(double dt);
    void setupInterface( Interface* interface, const std::string& name );
    
public:
    TimeController mTimeController;
};
