/*
 *  Pulsar.h
 *  Oculon
 *
 *  Created by Ehsan on 11-11-26.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __PULSAR_H__
#define __PULSAR_H__

#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include <vector>

#include "Scene.h"
#include "ParticleController.h"
#include "MidiMap.h"

using namespace ci;
using std::vector;

//
// Pulsar
//
class Pulsar : public Scene
{
public:
    Pulsar();
    virtual ~Pulsar();
    
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
    
    void drawStar();
    
private:
    
    //TODO: make a class for emitter/pulsar based on Body... maybe.
    // physical
    double mMass;
    ci::Quatd mRotation;    
    double mRotationVelocity;
    double mRadius;
    double mRotationAngle;
    
    // abstract
    float mHeat;
    int mFrameCount;
    
    
    gl::Texture		mEmitterTexture;
	//gl::Texture		mParticleTexture;
	gl::Texture		mNormalTexture;
	gl::Texture		mHeightTexture;
	gl::Texture		mSpecTexture;
    
    gl::GlslProg	mPulsarShader;
    
    ParticleController mParticleStream;
};

#endif // __PULSAR_H__
