//
//  ParticleFormation.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-03.
//
//

#pragma once

#include "cinder/gl/Texture.h"
#include "cinder/Surface.h"

class ParticleFormation
{
public:
    ParticleFormation(const std::string &name,
                      const int fboSize,
                      std::vector<ci::Vec4f> &positions,
                      std::vector<ci::Vec4f> &velocities,
                      std::vector<ci::Vec4f> &data);
    ParticleFormation(const std::string& name,
                      ci::gl::Texture posTex,
                      ci::gl::Texture velTex,
                      ci::gl::Texture dataTex);
    virtual ~ParticleFormation();
    
    const std::string& getName() const                  { return mName; }
    const ci::gl::Texture& getPositionTex() const       { return mPositionTex; }
    const ci::gl::Texture& getVelocityTex() const       { return mVelocityTex; }
    const ci::gl::Texture& getDataTex() const           { return mDataTex; }
    
protected:
    ParticleFormation(const std::string& name);
    void setTextures(ci::Surface32f& posSurface,
                     ci::Surface32f& velSurface,
                     ci::Surface32f& dataSurface);
    
    std::string mName;
    ci::gl::Texture mPositionTex;
    ci::gl::Texture mVelocityTex;
    ci::gl::Texture mDataTex;
};

#pragma mark - Formations

class RandomFormation : public ParticleFormation
{
public:
    RandomFormation(const int fboSize);
    RandomFormation(const int fboSize,
                    const ci::Vec4f& posMin     =ci::Vec4f::zero(),
                    const ci::Vec4f& posMax     =ci::Vec4f::one(),
                    const ci::Vec4f& velMin     =ci::Vec4f::zero(),
                    const ci::Vec4f& velMax     =ci::Vec4f::one(),
                    const ci::Vec4f& dataMin    =ci::Vec4f::zero(),
                    const ci::Vec4f& dataMax    =ci::Vec4f::one());
    ~RandomFormation() {}
};