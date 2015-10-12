//
//  ParticleFormationBehavior.cpp
//  Oculon1
//
//  Created by Ehsan Rezaie on 2015-10-11.
//
//

#include "ParticleFormationBehavior.h"
#include "Utils.h"

using namespace ci;
using namespace std;

void ParticleBehavior::setup()
{
    mShader = Utils::loadVertAndFragShaders("passThru_vert.glsl", "formation_sim_frag.glsl");
}

void ParticleBehavior::setupInterface(Interface *interface, const std::string &name)
{
    
}

void ParticleBehavior::update(double dt, PingPongFbo &particlesFbo)
{
    gl::pushMatrices();
    gl::setMatricesWindow( particlesFbo.getSize(), false ); // false to prevent vertical flipping
    gl::setViewport( particlesFbo.getBounds() );
    particlesFbo.bindUpdate();
    
    mShader.bind();
    
    mShader.uniform( "positions", 0 );
    mShader.uniform( "velocities", 1 );
    mShader.uniform( "information", 2);
    //    mShader.uniform( "dt", (float)dt );
    //    mShader.uniform( "reset", false );
    //    mShader.uniform( "formationStep", 0.0f );
    //    mShader.uniform( "motion", 0 );
    //    mShader.uniform( "containmentSize", 3.0f );
    
    gl::drawSolidRect(particlesFbo.getBounds());
    
    mShader.unbind();
    particlesFbo.unbindUpdate();
    gl::popMatrices();
}