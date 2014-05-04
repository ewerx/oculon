//
//  ParticleFormation.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-03.
//
//

#include "ParticleFormation.h"

using namespace std;
using namespace ci;

#pragma mark - construction

ParticleFormation::ParticleFormation(const std::string& name,
                  ci::gl::Texture posTex,
                  ci::gl::Texture velTex,
                  ci::gl::Texture dataTex)
: mName(name)
, mPositionTex(posTex)
, mVelocityTex(velTex)
, mDataTex(dataTex)
{
}

ParticleFormation::~ParticleFormation()
{
}

