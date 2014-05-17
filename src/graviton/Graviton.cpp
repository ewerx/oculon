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
    
    // params
    mTimeStep = 0.00075f;
    
    mFormationRadius = 80.0f;
    
    mDamping = 0.0f;
    mGravity = 0.05f;
    mEps = 0.0f;
    mConstraintSphereRadius = mFormationRadius * 1.25f;
    mNodeSpeed = 1.0f;
    
    mNumNodes = 0;
    mGravityNodeFormation = NODE_FORMATION_STATIC;
    
    // simulation
    mSimulationShader = loadFragShader("graviton_simulation_frag.glsl" );
    
    const int bufSize = 512;
    setupParticles(bufSize);
    
    // rendering
    mParticleController.addRenderer( new GravitonRenderer() );
    mParticleController.addRenderer( new LinesRenderer() );
    
    mCameraController.setup(mApp, 0, CameraController::CAM_SPLINE);
    mAudioInputHandler.setup(true);
    
    reset();
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
        velocities.push_back(Vec4f(0.0f, 0.0f, 0.0f, age));
        
        // extra info
        float decay = Rand::randFloat(.01f,10.00f);
        data.push_back(Vec4f(x,y,z,decay));
    }
    mParticleController.addFormation(new ParticleFormation("sphere", bufSize, positions, velocities, data));
    
    positions.clear();
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
//        
//        // extra info
//        float decay = Rand::randFloat(.01f,10.00f);
//        data.push_back(Vec4f(x,y,z,decay));
    }
    mParticleController.addFormation(new ParticleFormation("shell", bufSize, positions, velocities, data));
    
    
    positions.clear();
    // cube
    for (int i = 0; i < numParticles; ++i)
    {
        // position + mass
        float x = r * (Rand::randFloat()-0.5f);
        float y = r * (Rand::randFloat()-0.5f);
        float z = r * (Rand::randFloat()-0.5f);
        float mass = Rand::randFloat(0.01f,1.0f);
        positions.push_back(Vec4f(x,y,z,mass));
        
//        // velocity + age
//        //        float vx = Rand::randFloat(-.005f,.005f);
//        //        float vy = Rand::randFloat(-.005f,.005f);
//        //        float vz = Rand::randFloat(-.005f,.005f);
//        float age = Rand::randFloat(.007f,0.9f);
//        velocities.push_back(Vec4f(0.0f, 0.0f, 0.0f, age));
        
//        // extra info
//        float decay = Rand::randFloat(.01f,10.00f);
//        data.push_back(Vec4f(x,y,z,decay));
    }
    mParticleController.addFormation(new ParticleFormation("cube", bufSize, positions, velocities, data));
    
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
    
    vector<string> nodeFormationNames;
#define GRAVITON_NODE_FORMATION_ENTRY( nam, enm ) \
nodeFormationNames.push_back(nam);
    GRAVITON_NODE_FORMATION_TUPLE
#undef  GRAVITON_NODE_FORMATION_ENTRY
    mInterface->addEnum(CreateEnumParam( "Node Formation", (int*)(&mGravityNodeFormation) )
                        .maxValue(NODE_FORMATION_COUNT)
                        .oscReceiver(getName(), "nformation")
                        .isVertical(), nodeFormationNames)->registerCallback(this, &Graviton::resetGravityNodes);
    mInterface->addParam(CreateFloatParam( "Formation Radius", &mFormationRadius )
                         .minValue(10.0f)
                         .maxValue(1000.0f)
                         .oscReceiver(getName(), "formradius"));
    
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("simulation");
    mInterface->addParam(CreateFloatParam( "timestep", &mTimeStep )
                         .minValue(0.0f)
                         .maxValue(100.001f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "damping", &mDamping )
                         .maxValue(0.5f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "eps", &mEps )
                         .minValue(0.0)
                         .maxValue(0.00001)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "container-radius", &mConstraintSphereRadius )
                         .minValue(0.0f)
                         .maxValue(150.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "gravity", &mGravity )
                         .minValue(0.0f)
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "nodespeed", &mNodeSpeed )
                         .minValue(0.0f)
                         .maxValue(1000.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "audio-node", &mAudioMirror )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "random-mirror", &mRandomMirror )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "audio-gravity", &mAudioGravity )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "audio-container", &mAudioContainer )
                         .oscReceiver(getName()));
    
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("display");
    mParticleController.setupInterface(mInterface, mName);
    
    mCameraController.setupInterface(mInterface, mName);
    mAudioInputHandler.setupInterface(mInterface, mName);
}

void Graviton::reset()
{
    mReset = true;
    resetGravityNodes();
}

#pragma mark - Gravity Nodes

bool Graviton::resetGravityNodes()
{
    mGravityNodes.clear();
    
    const float r = mConstraintSphereRadius > 0.0f ? mConstraintSphereRadius : mFormationRadius;
    
    switch( mGravityNodeFormation )
    {
        case NODE_FORMATION_SPIN:
        {
            mNumNodes = 3;
            
            for(int i = 0; i < mNumNodes; ++i )
            {
                Vec3f pos;
                Vec3f vel = Vec3f::zero();
                
                vel.z = Rand::randFloat(0.01f, M_PI/3.0f);
                
                switch(i)
                {
                    case 0:
                    pos.x = r;
                    pos.y = 0.0f;
                    pos.z = 0.0f;
                    break;
                    
                    case 1:
                    pos.x = 0.0f;
                    pos.y = r;
                    pos.z = 0.0f;
                    break;
                    
                    case 2:
                    pos.x = 0.0f;
                    pos.y = 0.0f;
                    pos.z = r;
                    break;
                }
                
                const float mass = 10000.f;
                mGravityNodes.push_back( tGravityNode( pos, vel, mass ) );
            }
            
        }
        break;
        
        case NODE_FORMATION_MIRROR:
        {
            mNumNodes = 3;
            
            for(int i = 0; i < mNumNodes; ++i )
            {
                Vec3f pos;
                Vec3f vel;
                
                switch(i)
                {
                    case 0:
                    pos.x = 0.0f;
                    pos.y = 0.0f;
                    pos.z = 0.0f;
                    vel = Rand::randVec3f().normalized();
                    break;
                    
                    case 1:
                    pos.x = 0.0f;
                    pos.y = 0.0f;
                    pos.z = 0.0f;
                    vel = -1.0f * mGravityNodes[0].mVel;
                    break;
                    
                    default:
                    pos = Vec3f::one() * 1000000.f;
                    vel = Vec3f::zero();
                    break;
                    
                    //                    case 1:
                    //                        pos.x = r*0.5f*randFloat();
                    //                        pos.y = -r*0.5f*randFloat();
                    //                        pos.z = r / 4.0f*randFloat();
                    //                        vel = Rand::randVec3f();
                    //                        break;
                    //                    case 2:
                    //                        pos.x = -r*0.5f*randFloat();
                    //                        pos.y = r*0.5f*randFloat();
                    //                        pos.z = -r / 4.0f*randFloat();
                    //                        vel = Rand::randVec3f();
                    //                        break;
                }
                
                const float mass = Rand::randFloat(100000.f);
                mGravityNodes.push_back( tGravityNode( pos, vel, mass ) );
            }
        }
        break;
        
        case NODE_FORMATION_RANDOM:
        {
            mNumNodes = 1;
            
            for(int i = 0; i < mNumNodes; ++i )
            {
                Vec3f pos;
                Vec3f vel;
                
                switch(i)
                {
                    case 0:
                    pos.x = 0.0f;
                    pos.y = 0.0f;
                    pos.z = 0.0f;
                    vel = Rand::randVec3f();
                    break;
                    
                    default:
                    pos = Vec3f::one() * 1000000.f;
                    vel = Vec3f::zero();
                    break;
                    
                    //                    case 1:
                    //                        pos.x = r*0.5f*randFloat();
                    //                        pos.y = -r*0.5f*randFloat();
                    //                        pos.z = r / 4.0f*randFloat();
                    //                        vel = Rand::randVec3f();
                    //                        break;
                    //                    case 2:
                    //                        pos.x = -r*0.5f*randFloat();
                    //                        pos.y = r*0.5f*randFloat();
                    //                        pos.z = -r / 4.0f*randFloat();
                    //                        vel = Rand::randVec3f();
                    //                        break;
                }
                
                const float mass = Rand::randFloat(100000.f);
                mGravityNodes.push_back( tGravityNode( pos, vel, mass ) );
            }
        } break;
        
        
        case NODE_FORMATION_STATIC:
        {
            mNumNodes = 1;
            
            const float mass = 10000.f;
            Vec3f pos(0,0,0);
            for(int i = 0; i < mNumNodes; ++i )
            {
                mGravityNodes.push_back( tGravityNode( pos, Vec3f::zero(), mass ) );
            }
        } break;
        
        default:
        break;
    }
    
    return true;
}

void Graviton::updateGravityNodes(const double dt)
{
    float containRadius = mConstraintSphereRadius;
    if (mAudioContainer) {
        //containRadius *= mAudioInputHandler.getAverageVolumeMidFreq() * 10.0f;
    }
    
    for (int i = 0; i < mGravityNodes.size(); ++i)
    {
        switch (mGravityNodeFormation) {
            case NODE_FORMATION_RANDOM:
            {
                mGravityNodes[i].mPos += mGravityNodes[i].mVel * dt * 1000.f * mNodeSpeed;
                float distance = mGravityNodes[i].mPos.length();
                
                if (i == 0 && ((containRadius > 0.0f && distance > containRadius) || distance > 1000.f))
                {
                    mGravityNodes[i].mPos = mGravityNodes[i].mPos.normalized() * containRadius;
                    mGravityNodes[i].mVel.x *= Rand::randFloat(-1.25f, -0.75f);
                    mGravityNodes[i].mVel.y *= Rand::randFloat(-1.25f, -0.75f);
                    mGravityNodes[i].mVel.z *= Rand::randFloat(-1.25f, -0.75f);
                }
                
                if (i==0) {
                    //timeline().apply( &mCamTarget, mGravityNodes[i].mPos, 0.2f, EaseInQuad() );
                }
            }
            break;
            
            case NODE_FORMATION_MIRROR:
            {
                if (mAudioMirror)
                {
                    float distance = mAudioInputHandler.getAverageVolumeLowFreq() * mConstraintSphereRadius * 3.0f;
                    mGravityNodes[i].mPos = mGravityNodes[i].mVel * distance;
                    
                    // such hacks
                    if (i == 0 && mRandomMirror && distance > (containRadius*0.45f))
                    {
                        mGravityNodes[0].mVel = Rand::randVec3f().normalized();
                        mGravityNodes[1].mVel = mGravityNodes[0].mVel * -1.0f;
                    }
                }
                else
                {
                    mGravityNodes[i].mPos += mGravityNodes[i].mVel * dt * 1000.f * mNodeSpeed;
                    float distance = mGravityNodes[i].mPos.length();
                    
                    if (i != 2 && ((containRadius > 0.0f && distance > containRadius) || distance > 1000.f))
                    {
                        mGravityNodes[i].mPos = mGravityNodes[i].mPos.normalized() * containRadius;
                        mGravityNodes[i].mVel *= -1.0f;
                    }
                }
            }
            break;
            
            case NODE_FORMATION_SPIN:
            {
                if (containRadius > 0.0f) {
                    mGravityNodes[i].mVel.x += mGravityNodes[i].mVel.z * dt;
                    mGravityNodes[i].mVel.y += mGravityNodes[i].mVel.z * dt;
                    float theta = mGravityNodes[i].mVel.x;
                    float rho = mGravityNodes[i].mVel.y;
                    mGravityNodes[i].mPos = Vec3f(containRadius*cos(rho)*cos(theta), containRadius*sin(theta), containRadius*sin(rho)*cos(theta));
                }
            }
            break;
            
            default:
            break;
        }
    }
}

#pragma mark - Update

void Graviton::update(double dt)
{
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    mCameraController.update(dt);
    
    if (mAudioGravity)
    {
        mGravity = MIN(mAudioInputHandler.getAverageVolumeHighFreq(), 1.0f);
    }

    // update particle system
    updateGravityNodes(dt * 0.005f);
    
    // TODO: refactor
    PingPongFbo& mParticlesFbo = mParticleController.getParticleFbo();
    
    gl::pushMatrices();
    gl::disableAlphaBlending();
	gl::disableDepthRead();
	gl::disableDepthWrite();
    gl::setMatricesWindow( mParticlesFbo.getSize(), false ); // false to prevent vertical flipping
    gl::setViewport( mParticlesFbo.getBounds() );
    
    mParticlesFbo.bindUpdate();
    
    mParticleController.getFormation().getPositionTex().bind(3);
    mParticleController.getFormation().getVelocityTex().bind(4);
    
    mSimulationShader.bind();
    mSimulationShader.uniform( "positions", 0 );
    mSimulationShader.uniform( "velocities", 1 );
    mSimulationShader.uniform( "information", 2 );
	mSimulationShader.uniform( "oPositions", 3);
    mSimulationShader.uniform( "oVelocities", 4);
    mSimulationShader.uniform( "reset", mReset );
    mSimulationShader.uniform( "dt", (float)(dt * mTimeStep * 10.0f) );
    mSimulationShader.uniform( "eps", mEps );
    mSimulationShader.uniform( "damping", mDamping );
    mSimulationShader.uniform( "gravity", mGravity );
    float containRadius = mConstraintSphereRadius;
    if (mAudioContainer) {
        containRadius *= 0.25f + mAudioInputHandler.getAverageVolumeLowFreq() * 3.0f;
    }
    mSimulationShader.uniform( "containerradius", containRadius );
    mSimulationShader.uniform( "attractorPos1", mGravityNodes[0].mPos);
    mSimulationShader.uniform( "attractorPos2", mGravityNodes[1].mPos);
    mSimulationShader.uniform( "attractorPos3", mGravityNodes[2].mPos);
    mSimulationShader.uniform( "formationStep", mParticleController.getFormationStep() );
    
    gl::drawSolidRect(mParticlesFbo.getBounds());
    
    mSimulationShader.unbind();
    
    mParticleController.getFormation().getPositionTex().unbind();
    mParticleController.getFormation().getVelocityTex().unbind();
    
    mParticlesFbo.unbindUpdate();
    gl::popMatrices();
    
    mReset = false;
    
    Scene::update(dt);
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
    }
    gl::popMatrices();
}

void Graviton::drawDebug()
{
    Scene::drawDebug();
    
    gl::pushMatrices();
    gl::setMatrices( getCamera() );
    
    for (int i = 0; i < mGravityNodes.size(); ++i)
    {
        gl::color(1.0f, 0.0f, 0.0f);
        gl::drawSphere(mGravityNodes[i].mPos, 2.0f);
//        gl::enableWireframe();
//        gl::drawSphere(Vec3f::zero(), mConstraintSphereRadius);
//        gl::disableWireframe();
    }
    
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
