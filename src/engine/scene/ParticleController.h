/*
 *  ParticleController.h
 *  Oculon
 *
 *  Created by Ehsan on 11-11-26.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __PARTICLECONTROLLER_H__
#define __PARTICLECONTROLLER_H__

#include "Particle.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/gl/Texture.h"
#include "cinder/Perlin.h"
#include <list>

class Scene;


class ParticleController
{
public:
    enum eForce
    {
        FORCE_GRAVITY,
        FORCE_PERLIN,
        FORCE_REPULSION,
        FORCE_ATTRACTION,
        
        FORCE_COUNT
    };
    
    typedef std::list<Particle*> ParticleList;
    
public:
    ParticleController();
    ~ParticleController();
    
    void setup(Scene* owner);
    void update(double dt);
    void draw();
    
    void addParticles( int amt, ci::Vec3f pos, ci::Vec3f vel, float radius );
    void setForceEnabled( eForce force, bool enabled ) { mEnabledForces[force] = enabled; }
    
    int getParticleCount() const { return mParticles.size(); }
    
    void toggleParticleDrawMode() { mDrawAsBillboard = !mDrawAsBillboard; }
    
private:
    void applyForces(ParticleController::ParticleList::iterator p1, double dt );
    void applyForces2( double dt );
    
    Particle* getNewParticle();
 
private:
    enum { MAX_PARTICLES = 100000 };
    Particle*    mObjStore[MAX_PARTICLES];
    ParticleList mParticles;
    ParticleList mUnusedPool;
    
    bool mEnabledForces[FORCE_COUNT];
    ci::Perlin mPerlin;
    
    bool mDrawAsBillboard;
    
    int mCounter;
    ci::gl::Texture mParticleTexture;
    
    Scene* mScene;
};

#endif // __PARTICLECONTROLLER_H__
