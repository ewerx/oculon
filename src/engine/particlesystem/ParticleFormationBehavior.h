//
//  ParticleFormationBehavior.h
//  Oculon1
//
//  Created by Ehsan Rezaie on 2015-10-11.
//
//

#pragma once

#include "ParticleBehavior.h"

class ParticleFormationBehavior : public ParticleBehavior
{
public:
    ParticleFormationBehavior() : ParticleBehavior("take-formation") {}
    
    void setup();
    void setupInterface(Interface *interface, const std::string& name);
    void update(double dt, PingPongFbo &particleFbo);
    
protected:
    ci::Anim<float> mFormationStep;
    EaseCurveSelector mFormationAnimSelector;
    bool mStartAnim;
};
