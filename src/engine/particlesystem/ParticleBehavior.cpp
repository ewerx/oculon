//
//  ParticleBehavior.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2/20/2014.
//
//

#include "ParticleBehavior.h"
#include "Utils.h"

using namespace ci;


ParticleBehavior::ParticleBehavior()
{
    setup();
}

ParticleBehavior::~ParticleBehavior()
{
}

void ParticleBehavior::setup()
{
    mShader = Utils::loadVertAndFragShaders("passThru_vert.glsl", "static_sim_frag.glsl");
}

void ParticleBehavior::update(double dt, PingPongFbo &particlesFbo)
{
    gl::pushMatrices();
    gl::setMatricesWindow( particlesFbo.getSize(), false ); // false to prevent vertical flipping
    gl::setViewport( particlesFbo.getBounds() );
    particlesFbo.bindUpdate();
    
    mShader.uniform( "positions", 0 );
    mShader.uniform( "velocities", 1 );
    mShader.uniform( "information", 2);
    mShader.uniform( "dt", (float)dt );
    mShader.uniform( "reset", false );
    mShader.uniform( "formationStep", 0.0f );
    mShader.uniform( "motion", 0 );
    mShader.uniform( "containmentSize", 3.0f );
    
    gl::drawSolidRect(particlesFbo.getBounds());
    particlesFbo.unbindUpdate();
    gl::popMatrices();
}