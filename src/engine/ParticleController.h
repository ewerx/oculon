//
//  ParticleController.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2/11/2014.
//
//

#pragma once

#include "AudioInputHandler.h"
#include "Interface.h"
#include "PingPongFbo.h"
#include "ParticleBehavior.h"
#include "ParticleFormation.h"
#include "ParticleRenderer.h"

#include "cinder/Camera.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"

#include <vector>


class ParticleController
{
public:
    ParticleController();
    virtual ~ParticleController();
    
    void setup(int bufSize);
    void setupInterface( Interface* interface, const std::string& name );
    
    void update(double dt);
    void draw(const ci::Vec2i& screenSize, const ci::Camera& cam, AudioInputHandler& audioInputHandler);
    void drawDebug();
    
    // TODO: refactor so this is not needed
    PingPongFbo& getParticleFbo() { return mParticlesFbo; }
    
    // formations
    ParticleFormation& getFormation();
    void addFormation(ParticleFormation* formation);
    void addFormation(const std::string& name,
                      std::vector<ci::Vec4f>& positions,
                      std::vector<ci::Vec4f>& velocities,
                      std::vector<ci::Vec4f>& data);
    
    bool takeFormation(); // callback
    
    enum eResetFlag
    {
        RESET_POSITION    = (1<<0),
        RESET_VELOCITY    = (1<<1),
        RESET_DATA        = (1<<2),
        RESET_ALL         = RESET_POSITION | RESET_VELOCITY | RESET_DATA
    };
    void resetToFormation(const int formationIndex, const int resetFlags =RESET_ALL );
    const std::vector<std::string> getFormationNames();
    
    
    ParticleRenderer& getRenderer();
    void addRenderer(ParticleRenderer* renderer);
    const std::vector<std::string> getRendererNames();
    
private:
    void setupFBO();
    
    void updateSimulation(double dt);
    void render();
    
private:
    // particles
    PingPongFbo mParticlesFbo;
    
    int mFboSize;
    int mNumParticles;
    
    // formations
    std::vector<ParticleFormation*> mFormations;
    int mCurrentFormationIndex;
    
    // rendering
    std::vector<ParticleRenderer*> mRenderers;
    int mCurrentRendererIndex;
};
