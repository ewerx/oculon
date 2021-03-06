//
// Graviton.cpp
// Oculon
//
// Created by Ehsan on 11-11-26.
// Copyright 2011 ewerx. All rights reserved.
//


#include <boost/foreach.hpp>

#include "cinder/Rand.h"
#include "cinder/Easing.h"

#include "Graviton.h"
#include "OculonApp.h"
#include "Utils.h"
#include "Interface.h"
#include "GravitonRenderer.h"
#include "LinesRenderer.h"
#include "SpinCam.h"
#include "SplineCam.h"
#include "OtherSceneCam.h"

using namespace ci;
using namespace ci::app;
using namespace std;


#pragma mark - Construction

Graviton::Graviton()
: Scene("graviton")
{
}

Graviton::~Graviton()
{
}

#pragma mark - Setup

void Graviton::setup()
{
    Scene::setup();
    
    mReset = false;
    mDrawMoon = false;
    mGravitySync = false;
    mSpin = false;
    
    // params
//    mTimeStep = 0.01f;
    mTimeController.setTimeScale(0.1f);
    mTimeController.setTimeScaleMultiplier(20.0f);
    
    mFormationRadius = 80.0f;
    
    mDamping = 0.01f;
    mGravity = 0.5f;
    mGravity2 = mGravity;
    mEps = 0.0f;
    mConstraintSphereRadius = mFormationRadius * 1.25f;
    mAudioContainer = false;
    mAudioGravity = false;
    
    mHarmonic = Vec3f(2.0f, 4.0f, 8.0f);
    mHarmonicX = 0.5f;
    mHarmonicY = 1.0f;
    mHarmonicRate = 0.2f;
    
    // simulation
    mBehaviorSelector.addShader("take-formation", "formation_sim_frag.glsl");
    mBehaviorSelector.addShader("gravity", "graviton_simulation_frag.glsl" );
    mBehaviorSelector.addShader("spin", "graviton_spin_frag.glsl");
    mBehaviorSelector.addShader("harmonic", "harmonic_sim_frag.glsl");
    
    const int bufSize = 512;
    setupParticles(bufSize);
    
    const int nodeBufSize = 4;
    setupNodes(nodeBufSize);
    
    mNodeController.addRenderer( new GravitonRenderer() );
    
    mNodeBehaviorSelector.addShader("take-formation", "formation_sim_frag.glsl");
    mNodeBehaviorSelector.addShader("spin", "graviton_spin_frag.glsl");
    mNodeBehaviorSelector.addShader("harmonic", "harmonic_sim_frag.glsl");
    
    // rendering
    mParticleController.addRenderer( new GravitonRenderer() );
    //mParticleController.addRenderer( new LinesRenderer() );
    
    mCameraController.setup(mApp);
    mCameraController.addCamera( new SpinCam(mApp->getViewportAspectRatio()) );
    mCameraController.addCamera( new SplineCam(mApp->getViewportAspectRatio()) );
    mCameraController.addCamera( new OtherSceneCam(mApp, "lines") );
    mCameraController.addCamera( new OtherSceneCam(mApp, "parsec") );
    mCameraController.setCamIndex(1);
    
    mAudioInputHandler.setup(true);
    
//    MirrorBounceFormation* formation = new MirrorBounceFormation();
//    formation->mRadius = mFormationRadius * 3.0f;
//    formation->mBounceMultiplier = 50.0f;
//    formation->mSpinRate = 0.3f;
//    mNodeController.addFormation( formation );
//    
//    mNodeController.setFormation(1);
    
    reset();
}

void Graviton::setupNodes(const int bufSize)
{
    int numParticles = bufSize*bufSize;
    console() << "[graviton] initializing " << numParticles << " nodes, hang on!" << std::endl;
    
    mNodeController.setup(bufSize);
    
    const float r = mFormationRadius * 0.5f;
    
    vector<Vec4f> positions;
    vector<Vec4f> velocities;
    vector<Vec4f> data;
    
    // random
    {
        for (int i = 0; i < numParticles; ++i)
        {
            Vec3f pos = Rand::randVec3f() * r;
            
            float leadership = Rand::randFloat( 0.7f, 1.0f );	// GENERAL EMOTIONAL STATE.
            positions.push_back(Vec4f(pos.x,pos.y,pos.z,leadership));
            
            // velocity + age
            Vec3f vel = Rand::randVec3f() * 3.0f;
            float crowd = 1.0f;//Rand::randFloat(.007f,0.9f);
            velocities.push_back(Vec4f(vel.x,vel.y,vel.z,crowd));
            
            // extra info
            data.push_back(Vec4f::zero());
        }
        mNodeController.addFormation(new ParticleFormation("n-random", bufSize, positions, velocities, data));
    }
    
    positions.clear();
    velocities.clear();
    data.clear();
    
    // shell
    for (int i = 0; i < numParticles; ++i)
    {
        const float rho = Rand::randFloat() * (M_PI * 2.0);
        const float theta = Rand::randFloat() * (M_PI * 2.0);
        
        // position + mass
        float x = r * cos(rho) * sin(theta);
        float y = r * sin(rho) * sin(theta);
        float z = r * cos(theta);
        float mass = Rand::randFloat(0.01f,1.0f);
        positions.push_back(Vec4f(x,y,z,mass));
        
        //        // velocity + age
        //        //        float vx = Rand::randFloat(-.005f,.005f);
        //        //        float vy = Rand::randFloat(-.005f,.005f);
        //        //        float vz = Rand::randFloat(-.005f,.005f);
        //        float age = Rand::randFloat(.007f,0.9f);
        //        velocities.push_back(Vec4f(0.0f, 0.0f, 0.0f, age));
        
        float age = Rand::randFloat(.000001f,0.00005f);
        velocities.push_back(Vec4f(rho, theta, Rand::randFloat(-1.0f,1.0f), age));
        //
        // extra info
        float decay = Rand::randFloat(.01f,10.00f);
        data.push_back(Vec4f(x,y,z,decay));
    }
    mNodeController.addFormation(new ParticleFormation("n-shell", bufSize, positions, velocities, data));
}

void Graviton::setupParticles(const int bufSize)
{
	int numParticles = bufSize*bufSize;
    console() << "[graviton] initializing " << numParticles << " particles, hang on!" << std::endl;
    
    mParticleController.setup(bufSize);
    
    const float r = mFormationRadius;
    
    vector<Vec4f> positions;
    vector<Vec4f> velocities;
    vector<Vec4f> data;
    
    // sphere
    for (int i = 0; i < numParticles; ++i)
    {
        const float rho = Rand::randFloat() * (M_PI * 2.0);
        const float theta = Rand::randFloat() * (M_PI * 2.0);
        const float d = Rand::randFloat(10.0f, r);
        
        // position + mass
        float x = d * cos(rho) * sin(theta);
        float y = d * sin(rho) * sin(theta);
        float z = d * cos(theta);
        float mass = Rand::randFloat(0.01f,1.0f);
        positions.push_back(Vec4f(x,y,z,mass));
        
        // velocity + age
//        float vx = Rand::randFloat(-.005f,.005f);
//        float vy = Rand::randFloat(-.005f,.005f);
//        float vz = Rand::randFloat(-.005f,.005f);
        float age = Rand::randFloat(.007f,0.9f);
        velocities.push_back(Vec4f(rho, theta, Rand::randFloat(-1.0f,1.0f), age));
        
        // extra info
        float decay = Rand::randFloat(.01f,10.00f);
        data.push_back(Vec4f(x,y,z,decay));
    }
    mParticleController.addFormation(new ParticleFormation("sphere", bufSize, positions, velocities, data));
    
    positions.clear();
    velocities.clear();
    data.clear();
    // shell
    for (int i = 0; i < numParticles; ++i)
    {
        const float rho = Rand::randFloat() * (M_PI * 2.0);
        const float theta = Rand::randFloat() * (M_PI * 2.0);
        
        // position + mass
        float x = r * cos(rho) * sin(theta);
        float y = r * sin(rho) * sin(theta);
        float z = r * cos(theta);
        float mass = Rand::randFloat(0.01f,1.0f);
        positions.push_back(Vec4f(x,y,z,mass));
        
//        // velocity + age
//        //        float vx = Rand::randFloat(-.005f,.005f);
//        //        float vy = Rand::randFloat(-.005f,.005f);
//        //        float vz = Rand::randFloat(-.005f,.005f);
//        float age = Rand::randFloat(.007f,0.9f);
//        velocities.push_back(Vec4f(0.0f, 0.0f, 0.0f, age));
        
        float age = Rand::randFloat(.000001f,0.00005f);
        velocities.push_back(Vec4f(rho, theta, Rand::randFloat(-1.0f,1.0f), age));
//
        // extra info
        float decay = Rand::randFloat(.01f,10.00f);
        data.push_back(Vec4f(x,y,z,decay));
    }
    mParticleController.addFormation(new ParticleFormation("shell", bufSize, positions, velocities, data));
    
    
//    positions.clear();
//    data.clear();
//    // cube
//    for (int i = 0; i < numParticles; ++i)
//    {
//        // position + mass
//        float x = r * (Rand::randFloat()-0.5f);
//        float y = r * (Rand::randFloat()-0.5f);
//        float z = r * (Rand::randFloat()-0.5f);
//        float mass = Rand::randFloat(0.01f,1.0f);
//        positions.push_back(Vec4f(x,y,z,mass));
//        
////        // velocity + age
////        //        float vx = Rand::randFloat(-.005f,.005f);
////        //        float vy = Rand::randFloat(-.005f,.005f);
////        //        float vz = Rand::randFloat(-.005f,.005f);
////        float age = Rand::randFloat(.007f,0.9f);
////        velocities.push_back(Vec4f(0.0f, 0.0f, 0.0f, age));
//        
//        // extra info
//        float decay = Rand::randFloat(.01f,10.00f);
//        data.push_back(Vec4f(x,y,z,decay));
//    }
//    mParticleController.addFormation(new ParticleFormation("cube", bufSize, positions, velocities, data));
    
//    positions.clear();
//    data.clear();
//    // tight sphere
//    for (int i = 0; i < numParticles; ++i)
//    {
//        const float rho = Rand::randFloat() * (M_PI * 2.0);
//        const float theta = Rand::randFloat() * (M_PI * 2.0);
//        const float d = Rand::randFloat(10.0f, r * 0.25f);
//        
//        // position + mass
//        float x = d * cos(rho) * sin(theta);
//        float y = d * sin(rho) * sin(theta);
//        float z = d * cos(theta);
//        float mass = Rand::randFloat(0.01f,1.0f);
//        positions.push_back(Vec4f(x,y,z,mass));
//        
//        // velocity + age
//        //        float vx = Rand::randFloat(-.005f,.005f);
//        //        float vy = Rand::randFloat(-.005f,.005f);
//        //        float vz = Rand::randFloat(-.005f,.005f);
//        //float age = Rand::randFloat(.007f,0.9f);
//        //velocities.push_back(Vec4f(0.0f, 0.0f, 0.0f, age));
//        
//        // extra info
//        float decay = Rand::randFloat(.01f,10.00f);
//        data.push_back(Vec4f(x,y,z,decay));
//    }
//    mParticleController.addFormation(new ParticleFormation("sphere-small", bufSize, positions, velocities, data));
    
//    positions.clear();
//    data.clear();
//    // tight shell
//    for (int i = 0; i < numParticles; ++i)
//    {
//        const float rho = Rand::randFloat() * (M_PI * 2.0);
//        const float theta = Rand::randFloat() * (M_PI * 2.0);
//        
//        // position + mass
//        float x = r*0.25f * cos(rho) * sin(theta);
//        float y = r*0.25f * sin(rho) * sin(theta);
//        float z = r*0.25f * cos(theta);
//        float mass = Rand::randFloat(0.01f,1.0f);
//        positions.push_back(Vec4f(x,y,z,mass));
//        //
//        // extra info
//        float decay = Rand::randFloat(.01f,10.00f);
//        data.push_back(Vec4f(x,y,z,decay));
//    }
//    mParticleController.addFormation(new ParticleFormation("shell-small", bufSize, positions, velocities, data));
    
//    positions.clear();
//    velocities.clear();
//    data.clear();
//    // torus
//    for (int i = 0; i < numParticles; ++i)
//    {
//        const float rho = Rand::randFloat() * (M_PI * 2.0);
//        const float theta = Rand::randFloat() * (M_PI * 2.0);
//        
//        const float r2 = r * (0.25f + Rand::randFloat()*0.5f);
//        
//        // position + mass
//        float x = cos(theta) * (r + r2*cos(rho));
//        float y = sin(theta) * (r + r2*cos(rho));
//        float z = r2 * sin(rho);
//        float mass = Rand::randFloat(0.01f,1.0f);
//        positions.push_back(Vec4f(x,y,z,mass));
//        
//        float age = Rand::randFloat(.000001f,0.00005f);
//        velocities.push_back(Vec4f(rho, theta, Rand::randFloat(-1.0f,1.0f), age));
//        //
//        // extra info
//        float decay = Rand::randFloat(.01f,10.00f);
//        data.push_back(Vec4f(x,y,z,decay));
//    }
//    mParticleController.addFormation(new ParticleFormation("torus", bufSize, positions, velocities, data));
    
//    positions.clear();
//    data.clear();
//    // parabola?
//    for (int i = 0; i < numParticles; ++i)
//    {
//        const float rho = Rand::randFloat() * (M_PI * 2.0);
//        const float theta = Rand::randFloat() * (M_PI * 2.0);
//        
//        const float r2 = r * 0.5f;
//        
//        // position + mass
//        float x = cos(theta) * (r2 + r*cos(rho));
//        float y = sin(theta) * (r2 + r*cos(rho));
//        float z = r * sin(rho);
//        float mass = Rand::randFloat(0.01f,1.0f);
//        positions.push_back(Vec4f(x,y,z,mass));
//        //
//        // extra info
//        float decay = Rand::randFloat(.01f,10.00f);
//        data.push_back(Vec4f(x,y,z,decay));
//    }
//    mParticleController.addFormation(new ParticleFormation("parabola", bufSize, positions, velocities, data));
    
    // concentrics
    {
        positions.clear();
        data.clear();
        velocities.clear();
        
        const int NUM_CONCENTRIC = 4;
        const float multiplier = 1.0f / NUM_CONCENTRIC;
        float radii[NUM_CONCENTRIC];
        
        for (int j = 0; j < NUM_CONCENTRIC; ++j)
        {
            radii[j] = r * (1.0f - multiplier * j);
        }
        
        for (int i = 0; i < numParticles; ++i)
        {
            const float rho = Rand::randFloat() * (M_PI * 2.0);
            const float theta = Rand::randFloat() * (M_PI * 2.0);
            
            const int concentricIndex = i % NUM_CONCENTRIC;//Rand::randInt(NUM_CONCENTRIC);
            
            // position + mass
            float x = radii[concentricIndex] * cos(rho) * sin(theta);
            float y = radii[concentricIndex] * sin(rho) * sin(theta);
            float z = radii[concentricIndex] * cos(theta);
            float mass = 0.1f + multiplier * concentricIndex;
            positions.push_back(Vec4f(x,y,z,mass));
            
            // velocity + age
            float age = Rand::randFloat(.000001f,0.00005f);
            velocities.push_back(Vec4f(rho, theta, Rand::randFloat(-1.0f,1.0f), age));
            
            // extra info
            float decay = Rand::randFloat(.01f,10.00f);
            data.push_back(Vec4f(x,y,z,decay));
        }
        mParticleController.addFormation(new ParticleFormation("concentrics4", bufSize, positions, velocities, data));
    }
    
    // concentrics
    {
        positions.clear();
        data.clear();
        velocities.clear();
        
        const int NUM_CONCENTRIC = 8;
        const float multiplier = 1.0f / NUM_CONCENTRIC;
        float radii[NUM_CONCENTRIC];
        
        for (int j = 0; j < NUM_CONCENTRIC; ++j)
        {
            radii[j] = r * (1.0f - multiplier * j);
        }
        
        for (int i = 0; i < numParticles; ++i)
        {
            const float rho = Rand::randFloat() * (M_PI * 2.0);
            const float theta = Rand::randFloat() * (M_PI * 2.0);
            
            const int concentricIndex = i % NUM_CONCENTRIC;//Rand::randInt(NUM_CONCENTRIC);
            
            // position + mass
            float x = radii[concentricIndex] * cos(rho) * sin(theta);
            float y = radii[concentricIndex] * sin(rho) * sin(theta);
            float z = radii[concentricIndex] * cos(theta);
            float mass = 0.1f + multiplier * concentricIndex;
            positions.push_back(Vec4f(x,y,z,mass));
            
            // velocity + age
            float age = Rand::randFloat(.000001f,0.00005f);
            velocities.push_back(Vec4f(rho, theta, Rand::randFloat(-1.0f,1.0f), age));
            
            // extra info
            float decay = Rand::randFloat(.01f,10.00f);
            data.push_back(Vec4f(x,y,z,decay));
        }
        mParticleController.addFormation(new ParticleFormation("concentrics8", bufSize, positions, velocities, data));
    }
    
    // concentrics2
    {
        positions.clear();
        velocities.clear();
        data.clear();
        
        const int NUM_CONCENTRIC = 16;
        const float multiplier = 1.0f / NUM_CONCENTRIC;
        float radii[NUM_CONCENTRIC];
        for (int j = 0; j < NUM_CONCENTRIC; ++j)
        {
            radii[j] = r * (1.0f - multiplier * j);
        }
        for (int i = 0; i < numParticles; ++i)
        {
            const float rho = Rand::randFloat() * (M_PI * 2.0);
            const float theta = Rand::randFloat() * (M_PI * 2.0);
            
            const int concentricIndex = i % NUM_CONCENTRIC;//Rand::randInt(NUM_CONCENTRIC);
            
            // position + mass
            float x = radii[concentricIndex] * cos(rho) * sin(theta);
            float y = radii[concentricIndex] * sin(rho) * sin(theta);
            float z = radii[concentricIndex] * cos(theta);
            float mass = 0.1f + multiplier * concentricIndex;
            positions.push_back(Vec4f(x,y,z,mass));
            
            // velocity + age
            float age = Rand::randFloat(.000001f,0.00005f);
            velocities.push_back(Vec4f(rho, theta, Rand::randBool() ? 1.0f : 0.0f, age));
            
            // extra info
            float decay = Rand::randFloat(.01f,10.00f);
            data.push_back(Vec4f(x,y,z,decay));
        }
        mParticleController.addFormation(new ParticleFormation("concentrics16", bufSize, positions, velocities, data));
    }
    
    // lines
    {
        const int NUM_LINES = 256;
        const int POINTS_PER_LINE = numParticles / NUM_LINES;
        const float CENTER_RADIUS = 2.0f;
        const float POINT_SPACING = r / POINTS_PER_LINE;
        positions.clear();
        velocities.clear();
        data.clear();
        float rho = Rand::randFloat() * (M_PI);
        float theta = Rand::randFloat() * (M_PI * 2.0);
        float dist = -r;
        // lines
        for (int i = 0; i < numParticles; ++i)
        {
            if (i % POINTS_PER_LINE == 0)
            {
                rho = Rand::randFloat() * (M_PI);
                theta = Rand::randFloat() * (M_PI * 2.0f);
                dist = -r;
            }
            
            // position + mass
            float x = dist * cos(rho) * sin(theta);
            float y = dist * sin(rho) * sin(theta);
            float z = dist * cos(theta);
            float mass = Rand::randFloat(0.01f,1.0f);
            positions.push_back(Vec4f(x,y,z,mass));
            
            // velocity + age
            float age = Rand::randFloat(.000001f,0.00005f);
            velocities.push_back(Vec4f(rho, theta, Rand::randBool() ? 1.0f : 0.0f, age));
            
            //
            // extra info
            float decay = Rand::randFloat(.01f,10.00f);
            data.push_back(Vec4f(x,y,z,decay));
            
            dist += Rand::randFloat(POINT_SPACING*0.9f, POINT_SPACING*1.1f);
            dist = MIN(dist, -Rand::randFloat(CENTER_RADIUS*0.9f, CENTER_RADIUS*1.5f));
        }
        mParticleController.addFormation(new ParticleFormation("lines", bufSize, positions, velocities, data));
    }
    
    // rings
    {
        const int NUM_RINGS = 64;
        const int POINTS_PER_RING = numParticles / NUM_RINGS;
        positions.clear();
        velocities.clear();
        data.clear();
        float rho = Rand::randFloat() * (M_PI);
        float theta = Rand::randFloat() * (M_PI * 2.0);
        float distPer = r / NUM_RINGS;
        float dist = distPer;
        // lines
        for (int i = 0; i < numParticles; ++i)
        {
            rho = Rand::randFloat() * (M_PI * 2.0);
            
            if (i % POINTS_PER_RING == 0)
            {
                theta = Rand::randFloat() * (M_PI * 2.0f);
                dist += distPer;
            }
            
            float fuzzyTheta = theta * Rand::randFloat(0.95f, 1.05f);
            
            // position + mass
            float x = dist * cos(rho) * sin(fuzzyTheta);
            float y = dist * sin(rho) * sin(fuzzyTheta);
            float z = dist * cos(fuzzyTheta);
            float mass = Rand::randFloat(0.01f,1.0f);
            positions.push_back(Vec4f(x,y,z,mass));
            
            // velocity + age
            float age = Rand::randFloat(.000001f,0.00005f);
            velocities.push_back(Vec4f(rho, fuzzyTheta, 1.0f, age));
            
            // extra info
            float decay = Rand::randFloat(.01f,10.00f);
            data.push_back(Vec4f(x,y,z,decay));
        }
        mParticleController.addFormation(new ParticleFormation("rings", bufSize, positions, velocities, data));
    }
    
    // rings2
    {
        const int NUM_RINGS = 64;
        const int POINTS_PER_RING = numParticles / NUM_RINGS;
        bool flip = false;
        positions.clear();
        velocities.clear();
        data.clear();
        float rho = Rand::randFloat() * (M_PI);
        float theta = Rand::randFloat() * (M_PI * 2.0);
        float distPer = r / NUM_RINGS;
        float dist = distPer;
        // lines
        for (int i = 0; i < numParticles; ++i)
        {
            if (flip)
            {
                rho = Rand::randFloat() * (M_PI * 2.0f);
            }
            else
            {
                theta = Rand::randFloat() * (M_PI * 2.0f);
            }
            
            if (i % POINTS_PER_RING == 0)
            {
                if (flip)
                {
                    theta = Rand::randFloat() * (M_PI * 2.0f);
                }
                else
                {
                    rho = Rand::randFloat() * (M_PI * 2.0f);
                }
                dist += distPer;
                flip = !flip;
            }
            
            float finalRho = rho * Rand::randFloat(0.98f, 1.02f);
            float finalTheta = theta * Rand::randFloat(0.98f, 1.02f);
            
            // position + mass
            float x = dist * cos(finalRho) * sin(finalTheta);
            float y = dist * sin(finalRho) * sin(finalTheta);
            float z = dist * cos(finalTheta);
            float mass = Rand::randFloat(0.01f,1.0f);
            positions.push_back(Vec4f(x,y,z,mass));
            
            // velocity + age
            float age = Rand::randFloat(.00001f,0.0005f);
            velocities.push_back(Vec4f(finalRho, finalTheta, flip ? 1.0f : 0.0f, age));
            
            // extra info
            float decay = Rand::randFloat(.01f,10.00f);
            data.push_back(Vec4f(x,y,z,decay));
        }
        mParticleController.addFormation(new ParticleFormation("rings2", bufSize, positions, velocities, data));
    }
    
    // cube
    {
        const int d = 64;
        positions.clear();
        velocities.clear();
        data.clear();
        float distPer = r / (d*0.5);
        
        float x = -0.5 * d * distPer;
        float y = -0.5 * d * distPer;
        float z = -0.5 * d * distPer;
        
        // lines
        for (int i = 0; i < d; ++i)
        {
            z = -0.5 * d * distPer;
            for (int j = 0; j < d; ++j)
            {
                x = -0.5 * d * distPer;
                for (int k = 0; k < d; ++k)
                {
                    
                    float mass = Rand::randFloat(0.01f,1.0f);
                    positions.push_back(Vec4f(x,y,z,mass));
                    
                    // velocity + age
                    float age = Rand::randFloat(.000001f,0.00005f);
                    float speed = 0.01f * (Rand::randFloat(-1.0f, 1.0f));
                    velocities.push_back(Vec4f(x*speed, y*speed, z*speed, age));
                    
                    // extra info
                    float decay = Rand::randFloat(.01f,10.00f);
                    data.push_back(Vec4f(x,y,z,decay));
                    
                    x += distPer;
                }
                
                z += distPer;
            }
            
            y += distPer;
        }
        mParticleController.addFormation(new ParticleFormation("cube", bufSize, positions, velocities, data));
    }
    
    mParticleController.resetToFormation(0);
}

//void Graviton::setupDebugInterface()
//{
//    Scene::setupDebugInterface(); // add all interface params
//    
//}

void Graviton::setupInterface()
{
    mInterface->gui()->addSeparator();
    
    mTimeController.setupInterface(mInterface, getName(), 1, 22);
    
    mInterface->addEnum(CreateEnumParam("behavior", &mBehaviorSelector.mIndex)
                        .maxValue(mBehaviorSelector.mNames.size())
                        .isVertical()
                        .sendFeedback(), mBehaviorSelector.mNames)->registerCallback(&mParticleController, &ParticleController::onFormationChanged);
    
    //mInterface->addParam(CreateVec3fParam("harmonic", &mHarmonic, Vec3f::zero(), Vec3f::one() * 16.0f));
    mInterface->addParam(CreateFloatParam("harmonc-X", &mHarmonicX )
                         .maxValue(8.0f));
    mInterface->addParam(CreateFloatParam("harmonc-Y", &mHarmonicY )
                         .maxValue(8.0f));
    mInterface->addParam(CreateFloatParam("harmonc-rate", &mHarmonicRate )
                         .minValue(0.01f)
                         .maxValue(5.0f));
                         
    mInterface->gui()->addColumn();
    mInterface->addEnum(CreateEnumParam("node-behavior", &mNodeBehaviorSelector.mIndex)
                        .maxValue(mNodeBehaviorSelector.mNames.size())
                        .isVertical()
                        .sendFeedback(), mNodeBehaviorSelector.mNames)->registerCallback(&mNodeController, &ParticleController::onFormationChanged);
    mInterface->gui()->addSeparator();
    mNodeController.setupInterface(mInterface, getName());
    
//    mInterface->addParam(CreateFloatParam( "Formation Radius", &mFormationRadius )
//                         .minValue(10.0f)
//                         .maxValue(1000.0f)
//                         .oscReceiver(getName(), "formradius"));
    
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("simulation");
    mInterface->addParam(CreateBoolParam( "spin", &mSpin )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "damping", &mDamping )
                         .maxValue(0.2f)
                         .oscReceiver(getName())
                         .midiInput(0, 1, 27));
    mInterface->addParam(CreateFloatParam( "eps", &mEps )
                         .minValue(0.0)
                         .maxValue(0.00001)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "container-radius", &mConstraintSphereRadius )
                         .minValue(0.0f)
                         .maxValue(250.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "gravity", &mGravity )
                         .minValue(0.0f)
                         .maxValue(20.0f)
                         .oscReceiver(getName())
                         .midiInput(0, 1, 25));
//    mInterface->addParam(CreateBoolParam( "sync-gravity", &mGravitySync )
//                         .oscReceiver(getName()));
//    mInterface->addParam(CreateFloatParam( "gravity2", &mGravity2 )
//                         .minValue(0.0f)
//                         .maxValue(10.0f)
//                         .oscReceiver(getName())
//                         .midiInput(0, 2, 25));
//    mInterface->addParam(CreateFloatParam( "nodespeed", &mNodeSpeed )
//                         .minValue(0.0f)
//                         .maxValue(1000.0f)
//                         .oscReceiver(getName()));
//    mInterface->addParam(CreateBoolParam( "audio-node", &mAudioMirror )
//                         .oscReceiver(getName()));
//    mInterface->addParam(CreateBoolParam( "random-mirror", &mRandomMirror )
//                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "audio-gravity", &mAudioGravity )
                         .oscReceiver(getName())
                         .midiInput(0, 2, 27));
    mInterface->addParam(CreateBoolParam( "audio-container", &mAudioContainer )
                         .oscReceiver(getName())
                         .midiInput(0, 2, 26));
    
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("render");
    mParticleController.setupInterface(mInterface, getName());
    
    mCameraController.setupInterface(mInterface, getName());
    mAudioInputHandler.setupInterface(mInterface, getName(), 1, 23);
    
    // FIXME: MIDI HACK
    mowa::sgui::PanelControl* hiddenPanel = mInterface->gui()->addPanel();
    hiddenPanel->enabled = false;
    mInterface->addButton(CreateTriggerParam("formation0", NULL)
                          .midiInput(0, 2, 21))->registerCallback( boost::bind( &ParticleController::setFormation, &mParticleController, 0) );
    mInterface->addButton(CreateTriggerParam("formation1", NULL)
                          .midiInput(0, 2, 22))->registerCallback( boost::bind( &ParticleController::setFormation, &mParticleController, 1) );
    mInterface->addParam(CreateFloatParam("anim_time", mParticleController.getAnimTimePtr())
                         .minValue(0.0f)
                         .maxValue(120.0f)
                         .midiInput(0, 1, 21));
    
    mInterface->addParam(CreateBoolParam( "moon", &mDrawMoon )
                         .oscReceiver(getName()));
}

void Graviton::reset()
{
    mReset = true;
}

#pragma mark - Update

void Graviton::update(double dt)
{
    mTimeController.update(dt);
    
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    mCameraController.update(dt);
    
    gl::disableAlphaBlending();
	gl::disableDepthRead();
	gl::disableDepthWrite();
    
    updateNodes(dt);
    updateParticles(dt);
    
    mReset = false;
    
    Scene::update(dt);
}

void Graviton::updateParticles(double dt)
{
    PingPongFbo& fbo = mParticleController.getParticleFbo();
    
    gl::pushMatrices();
    gl::setMatricesWindow( fbo.getSize(), false ); // false to prevent vertical flipping
    gl::setViewport( fbo.getBounds() );
    
    fbo.bindUpdate();
    
    mParticleController.getFormation().getPositionTex().bind(3);
    mParticleController.getFormation().getVelocityTex().bind(4);
    
    mNodeController.getParticleFbo().bindTexture(5, 0);
    
    float gravity = mGravity;
    float gravity2 = mGravitySync ? mGravity : mGravity2;
    if (mAudioGravity)
    {
//        if (mNodeController.getNodes().size() > 0)
//        {
//            gravity *= mNodeController.getNodes().front().mPosition.length() * 0.25f;
//            gravity2 *= mNodeController.getNodes().back().mPosition.length() * 0.25f;
//        }
//        else
        {
            gravity *= mAudioInputHandler.getAverageVolumeLowFreq() * 5.0f;
            gravity2 *= mAudioInputHandler.getAverageVolumeLowFreq() * 5.0f;
        }
    }
    
    gl::GlslProg shader = mBehaviorSelector.getSelectedShader();
    
    shader.bind();
    shader.uniform( "positions", 0 );
    shader.uniform( "velocities", 1 );
    shader.uniform( "information", 2 );
    shader.uniform( "oPositions", 3);
    shader.uniform( "oVelocities", 4);
    shader.uniform( "nodePosTex", 5 );
    shader.uniform( "nodeBufSize", (float)mNodeController.getFboSize() );
    shader.uniform( "reset", mReset );
    shader.uniform( "spin", mSpin );
    shader.uniform( "dt", (float)mTimeController.getDelta() );
    shader.uniform( "eps", mEps );
    shader.uniform( "damping", mDamping );
    shader.uniform( "gravity", gravity );
    shader.uniform( "gravity2", gravity2 );
    float containRadius = mConstraintSphereRadius;
    if (mAudioContainer) {
        containRadius *= 0.25f + mAudioInputHandler.getAverageVolumeLowFreq() * 3.0f;
    }
    shader.uniform( "containerradius", containRadius );
//    shader.uniform( "harmonic", mHarmonic );
    shader.uniform( "harmonicX", mHarmonicX );
    shader.uniform( "harmonicY", mHarmonicY );
    shader.uniform( "harmonicRate", mHarmonicRate );
    
//    NodeFormation::tNodeList& nodes = mNodeController.getNodes();
//    // TODO: glsl array uniform?
//    if (nodes.size() > 0) shader.uniform( "attractorPos1", nodes[0].mPosition );
//    if (nodes.size() > 1) shader.uniform( "attractorPos2", nodes[1].mPosition );
//    if (nodes.size() > 2) shader.uniform( "attractorPos3", nodes[2].mPosition );
//    if (nodes.size() > 3) shader.uniform( "attractorPos4", nodes[3].mPosition );
//    
//    shader.uniform( "attractorMass1", nodes.size() > 0 ? 1.0f : 0.0f );
//    shader.uniform( "attractorMass2", nodes.size() > 1 ? 1.0f : 0.0f );
//    shader.uniform( "attractorMass3", nodes.size() > 2 ? 1.0f : 0.0f );
//    shader.uniform( "attractorMass4", nodes.size() > 3 ? 1.0f : 0.0f );
    
    shader.uniform( "startAnim", mParticleController.isStartingAnim() );
    shader.uniform( "formationStep", mParticleController.getFormationStep() );
    
    gl::drawSolidRect(fbo.getBounds());
    
    shader.unbind();
    
    mNodeController.getParticleFbo().unbindTexture();
    
    mParticleController.getFormation().getPositionTex().unbind();
    mParticleController.getFormation().getVelocityTex().unbind();
    
    fbo.unbindUpdate();
    gl::popMatrices();
}

void Graviton::updateNodes(double dt)
{
    // TODO: refactor
    PingPongFbo& fbo = mNodeController.getParticleFbo();
    
    gl::pushMatrices();
    gl::setMatricesWindow( fbo.getSize(), false ); // false to prevent vertical flipping
    gl::setViewport( fbo.getBounds() );
    
    fbo.bindUpdate();
    
    mNodeController.getFormation().getPositionTex().bind(3);
    mNodeController.getFormation().getVelocityTex().bind(4);
    
    //    mDynamicTexture.bindTexture(5);
    //
    //    if (mAudioInputHandler.hasTexture())
    //    {
    //        mAudioInputHandler.getFbo().bindTexture(6);
    //    }
    
    gl::GlslProg shader = mNodeBehaviorSelector.getSelectedShader();
    
    float simdt = mTimeController.getDelta();
    //if (mAudioTime) simdt *= (1.0 - mAudioInputHandler.getAverageVolumeLowFreq());
    shader.bind();
    shader.uniform( "positions", 0 );
    shader.uniform( "velocities", 1 );
    shader.uniform( "information", 2);
    shader.uniform( "oPositions", 3);
    shader.uniform( "oVelocities", 4);
    //  	shader.uniform( "noiseTex", 5);
    //    shader.uniform( "audioData", 6);
    //    shader.uniform( "gain", mAudioInputHandler.getGain());
    shader.uniform( "dt", (float)simdt );
    shader.uniform( "reset", mReset );
    shader.uniform( "startAnim", mNodeController.isStartingAnim() );
    shader.uniform( "formationStep", mNodeController.getFormationStep() );
    //    shader.uniform( "containmentRadius", mContainmentRadius );
    
    gl::drawSolidRect(fbo.getBounds());
    
    shader.unbind();
    
    //mDynamicTexture.unbindTexture();
    
    mNodeController.getFormation().getPositionTex().unbind();
    mNodeController.getFormation().getVelocityTex().unbind();
    
    fbo.unbindUpdate();
    gl::popMatrices();
}

#pragma mark - Input

//void Graviton::updateNeuralResponse()
//{
//    MindWave& mindWave = mApp->getMindWave();
//    if( mindWave.hasData() )
//    {
//        if( mindWave.getBlink() > 0.0f )
//        {
//            if( mResetCameraByBlink )
//            {
//                setupCameraSpline();
//            }
//        }
//        
//        if( mColorByMindWave )
//        {
////            const float attention = mindWave.getAttention();
////            const float meditation = mindWave.getMeditation();
//            
//            
//        }
//    }
//}

#pragma mark - Camera

const Camera& Graviton::getCamera()
{
    if (mApp->outputToOculus())
    {
        return mApp->getOculusCam().getCamera();
    }
    else
    {
        return mCameraController.getCamera();
    }
}

#pragma mark - Draw

void Graviton::draw()
{
    gl::pushMatrices();
    if (mApp->outputToOculus())
    {
        // render left eye
        Area leftViewport = Area( Vec2f( 0.0f, 0.0f ), Vec2f( getFbo().getWidth() / 2.0f, getFbo().getHeight() ) );
        gl::setViewport(leftViewport);
        mApp->getOculusCam().enableStereoLeft();
        mParticleController.draw(leftViewport.getSize(), mApp->getOculusCam().getCamera(), mAudioInputHandler);
        
        Area rightViewport = Area( Area( Vec2f( getFbo().getWidth() / 2.0f, 0.0f ), Vec2f( getFbo().getWidth(), getFbo().getHeight() ) ) );
        gl::setViewport(rightViewport);
        mApp->getOculusCam().enableStereoRight();
        mParticleController.draw(rightViewport.getSize(), mApp->getOculusCam().getCamera(), mAudioInputHandler);
    }
    else
    {
        mParticleController.draw(mApp->getViewportSize(), getCamera(), mAudioInputHandler);
        mNodeController.draw(mApp->getViewportSize(), getCamera(), mAudioInputHandler);
        
        // draw moon
//        glPushAttrib( GL_ALL_ATTRIB_BITS );
//        
//        glShadeModel( GL_SMOOTH );
//        gl::enable( GL_POLYGON_SMOOTH );
//        glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
//        gl::enable( GL_NORMALIZE );
//        gl::enableAlphaBlending();
//        gl::enableDepthRead();
//        gl::enableDepthWrite();
//        
//        glDepthFunc( GL_LESS );
//        glEnable( GL_DEPTH_TEST );
//        
//        glCullFace( GL_BACK );
//        glEnable( GL_CULL_FACE );
        
        if (mDrawMoon)
        {
        gl::pushMatrices();
        // Set up window
        gl::setViewport( mApp->getViewportBounds() );
        gl::setMatrices( getCamera() );
        mMoon.draw();
        gl::popMatrices();
//        glPopAttrib();
        }
        
    }
    gl::popMatrices();
}

void Graviton::drawDebug()
{
    Scene::drawDebug();
    
    gl::pushMatrices();
    gl::setMatrices( getCamera() );
    
//    for (int i = 0; i < mGravityNodes.size(); ++i)
//    {
//        gl::color(1.0f, 0.0f, 0.0f);
//        gl::drawSphere(mGravityNodes[i].mPos, 2.0f);
////        gl::enableWireframe();
////        gl::drawSphere(Vec3f::zero(), mConstraintSphereRadius);
////        gl::disableWireframe();
//    }
//    mNodeController.drawDebug();
    
    gl::popMatrices();
    
    gl::pushMatrices();
    glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
    gl::enable( GL_TEXTURE_2D );
    const Vec2f& windowSize( getWindowSize() );
    gl::setMatricesWindow( windowSize );
    gl::color(ColorA::white());
    
    const float size = 80.0f;
    const float paddingX = 20.0f;
    const float paddingY = 240.0f;
    
    //HACK
    PingPongFbo& mParticlesFbo = mParticleController.getParticleFbo();
    
    Rectf preview( windowSize.x - (size+paddingX), windowSize.y - (size+paddingY), windowSize.x-paddingX, windowSize.y - paddingY );
    
    gl::draw( mParticlesFbo.getTexture(0), preview );
    
    Rectf preview2 = preview - Vec2f(size+paddingX, 0.0f);
    gl::draw( mParticlesFbo.getTexture(1), preview2 );
    
    glPopAttrib();
    gl::popMatrices();
    
    
    //drawCamSpline();
}
