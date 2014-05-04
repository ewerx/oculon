//
//  ParticleFormation.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-03.
//
//

#pragma once

#include "cinder/gl/Texture.h"

class ParticleFormation
{
public:
    ParticleFormation(const std::string& name,
                      ci::gl::Texture posTex,
                      ci::gl::Texture velTex,
                      ci::gl::Texture dataTex);
    ~ParticleFormation();
    
    const std::string& getName() const                  { return mName; }
    const ci::gl::Texture& getPositionTex() const       { return mPositionTex; }
    const ci::gl::Texture& getVelocityTex() const       { return mVelocityTex; }
    const ci::gl::Texture& getDataTex() const           { return mDataTex; }
    
private:
    std::string mName;
    ci::gl::Texture mPositionTex;
    ci::gl::Texture mVelocityTex;
    ci::gl::Texture mDataTex;
};
