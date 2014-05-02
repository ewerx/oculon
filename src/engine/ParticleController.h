//
//  ParticleController.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2/11/2014.
//
//

#pragma once

#include "PingPongFbo.h"
#include "ParticleBehavior.h"
#include "ParticleRenderer.h"

#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"

#include <vector>


class ParticleController
{
public:
    struct tFormation
    {
        tFormation(const std::string& name,
                   ci::gl::Texture posTex,
                   ci::gl::Texture velTex,
                   ci::gl::Texture dataTex)
        : mName(name)
        , mPositionTex(posTex)
        , mVelocityTex(velTex)
        , mDataTex(dataTex)
        {}
        
        std::string mName;
        ci::gl::Texture mPositionTex;
        ci::gl::Texture mVelocityTex;
        ci::gl::Texture mDataTex;
    };
public:
    ParticleController();
    virtual ~ParticleController();
    
    void setup(int bufSize);
    
    void update(double dt);
    void draw(const ci::Camera& cam);
    void drawDebug();
    
    void addFormation(const std::string& name,
                      std::vector<ci::Vec4f>& positions,
                      std::vector<ci::Vec4f>& velocities,
                      std::vector<ci::Vec4f>& data);
    
    enum eResetFlag
    {
        RESET_POSITION    = (1<<0),
        RESET_VELOCITY    = (1<<1),
        RESET_DATA        = (1<<2),
        RESET_ALL         = RESET_POSITION | RESET_VELOCITY | RESET_DATA
    };
    void resetToFormation(const int formationIndex, const int resetFlags =RESET_ALL );
    
    PingPongFbo& getParticleFbo() { return mParticlesFbo; }
    int getFormationCount() { return mFormations.size(); }
    
    std::vector<tFormation>& getFormations() { return mFormations; }
    
public:
    //void addBehavior(ParticleBehavior* behavior) { mBehaviors.push_back(behavior); }
    
private:
    void setupFBO();
    
    void updateSimulation(double dt);
    void render();
    
private:
    PingPongFbo mParticlesFbo;
    
    int mFboSize;
    int mNumParticles;

    std::vector<tFormation> mFormations;
};
