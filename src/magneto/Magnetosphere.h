/*
 *  Magnetosphere.h
 *  Oculon
 *
 *  Created by Ehsan on 11-11-26.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __MAGNETOSPHERE_H__
#define __MAGNETOSPHERE_H__

#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include <vector>

#include "Scene.h"
#include "ParticleController.h"
#include "MidiMap.h"

using namespace ci;
using std::vector;

//
// The Magnificent Magnetosphere!
//
class Magnetosphere : public Scene
{
public:
    Magnetosphere();
    virtual ~Magnetosphere();
    
    // inherited from Scene
    void setup();
    void setupParams(params::InterfaceGl& params);
    void reset();
    void resize();
    void update(double dt);
    void draw();
    bool handleKeyDown(const KeyEvent& keyEvent);
    
private:
    void updateAudioResponse();
    void generateParticles();
    
private:
    
    ParticleController mParticleController;
};

#endif // __MAGNETOSPHERE_H__
