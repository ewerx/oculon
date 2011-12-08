/*
 *  Magnetosphere.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-11-26.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Magnetosphere.h"

#include "AudioInput.h" // compile errors if this is not before App.h
#include "OculonApp.h"//TODO: fix this dependency
#include "cinder/Rand.h"

using namespace ci;

Magnetosphere::Magnetosphere()
: Scene()
{
}

Magnetosphere::~Magnetosphere()
{
}

void Magnetosphere::setup()
{
    mParticleController.setup();
    reset();
}

void Magnetosphere::reset()
{
}

void Magnetosphere::resize()
{
}

//void Magnetosphere::setupMidiMapping()
//{
    // setup MIDI inputs for learning
    //mMidiMap.registerMidiEvent("orb_gravity", MidiEvent::TYPE_VALUE_CHANGE, this, &Orbiter::handleGravityChange);
    //mMidiMap.registerMidiEvent("orb_timescale", MidiEvent::TYPE_VALUE_CHANGE, this, &Orbiter::handleGravityChange);
    //mMidiMap.beginLearning();
    // ... or load a MIDI mapping
    //mMidiInput.setMidiKey("gravity", channel, note);
//}

void Magnetosphere::setupParams(params::InterfaceGl& params)
{
    params.addText( "magneto", "label=`Magnetosphere`" );
    //params.addParam("Gravity Constant", &mGravityConstant, "step=0.00000000001 keyIncr== keyDecr=-");
    //params.addParam("Follow Target", &mFollowTargetIndex, "keyIncr=] keyDecr=[");
    //params.addParam("Time Scale", &mTimeScale, "step=86400.0 KeyIncr=. keyDecr=,");
    //params.addParam("Max Radius Mult", &Orbiter::sMaxRadiusMultiplier, "step=0.1");
    //params.addParam("Frames to Avg", &Orbiter::sNumFramesToAvgFft, "step=1");
    //params.addParam("Trails - Smooth", &Orbiter::sUseSmoothLines, "key=s");
    //params.addParam("Trails - Ribbon", &Orbiter::sUseTriStripLine, "key=t");
    //params.addParam("Trails - LengthFact", &Orbiter::sMinTrailLength, "keyIncr=l keyDecr=;");
    //params.addParam("Trails - Width", &Orbiter::sTrailWidth, "keyIncr=w keyDecr=q step=0.1");
    //params.addParam("Planet Grayscale", &Orbiter::sPlanetGrayScale, "keyIncr=x keyDecr=z step=0.05");
    //params.addParam("Real Sun Radius", &Orbiter::sDrawRealSun, "key=r");
    //params.addSeparator();
    //params.addParam("Frustum Culling", &mEnableFrustumCulling, "keyIncr=f");
}

void Magnetosphere::update(double dt)
{
    mParticleController.update(dt);
    
    updateAudioResponse();
    //updateHud();
    
    Scene::update(dt);
    
    // debug info
    char buf[256];
    snprintf(buf, 256, "particles: %d", mParticleController.getParticleCount());
    mApp->getInfoPanel().addLine(buf, Color(0.75f, 0.5f, 0.5f));
}

//
// handleKeyDown
//
bool Magnetosphere::handleKeyDown(const KeyEvent& keyEvent)
{
    bool handled = true;
    
    switch (keyEvent.getChar()) 
    {
        case ' ':
            reset();
            break;
        case 'g':
            generateParticles();
            break;
        default:
            handled = false;
            break;
    }
    
    return handled;
}

//
//
//
void Magnetosphere::updateAudioResponse()
{
    AudioInput& audioInput = mApp->getAudioInput();
    std::shared_ptr<float> fftDataRef = audioInput.getFftDataRef();
    
    //unsigned int bandCount = audioInput.getFftBandCount();
    //float* fftBuffer = fftDataRef.get();
    
    //int bodyIndex = 0;
    
    //TODO
}

void Magnetosphere::draw()
{
    mParticleController.draw();
}

void Magnetosphere::generateParticles()
{
    Vec3f pos = Vec3f::zero();
    Vec3f vel = Vec3f::zero();
    float emitterRadius = 5.0f;
    int depth = pos.y - 380;
    float per = depth/340.0f;
    //Vec3f vel = mEmitter.mVel * per;
    //vel.y *= 0.02f;
    int numParticlesToSpawn = Rand::randInt(100,250);
    if( Rand::randFloat() < 0.02f )
    {
        //numParticlesToSpawn *= 5;
    }
    mParticleController.addParticles( numParticlesToSpawn, pos, vel * per, emitterRadius );
}
