//
//  SimplexNoiseTexture.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-04-29.
//
//

#include "SimplexNoiseTexture.h"
#include "Utils.h"

using namespace ci;

SimplexNoiseTexture::SimplexNoiseTexture()
{
}

SimplexNoiseTexture::~SimplexNoiseTexture()
{
}

void SimplexNoiseTexture::initShader()
{
    mShader = Utils::loadFragShader("simplex_frag.glsl");
    
    // params
    mNoiseTheta = 0.0f;
    mNoiseSpeed = 0.1f;
    mNoiseScale = Vec3f(1.0f,1.0f,0.25f);
}

void SimplexNoiseTexture::configShader(double dt)
{
    mNoiseTheta += dt * mNoiseSpeed;
    
    mShader.uniform( "theta", mNoiseTheta );
    mShader.uniform( "scale", mNoiseScale );
}

void SimplexNoiseTexture::setupInterface(Interface* interface, const std::string &name)
{
    interface->gui()->addLabel("simplex");
    interface->addParam(CreateFloatParam("noise_speed", &mNoiseSpeed )
                       .maxValue(1.0f)
                       .oscReceiver(name));
    interface->addParam(CreateVec3fParam("noise", &mNoiseScale, Vec3f::zero(), Vec3f(10.0f,10.0f,1.0f))
                       .oscReceiver(name));
}