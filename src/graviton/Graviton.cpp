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
    
    mTimeStep = 0.00075f;
        
    mInitialFormation = FORMATION_SPHERE;
    mFormationRadius = 80.0f;
    
    mAdditiveBlending = true;
    mUseImageForPoints = true;
    mPointSize = 0.6f;
    mColorScale = ColorAf( 0.5f, 0.5f, 0.6f, 0.5f );
    
    mDamping = 0.0f;
    mGravity = 0.05f;
    mEps = 0.0f;
    mConstraintSphereRadius = 100.0f;
    mNodeSpeed = 1.0f;
    
    mNumNodes = 0;
    mGravityNodeFormation = NODE_FORMATION_STATIC;
    
    // shaders
    mSimulationShader = loadFragShader("graviton_simulation_frag.glsl" );
    mRenderShader = loadVertAndFragShaders("graviton_render_vert.glsl",  "graviton_render_frag.glsl");
    mFormationShader = loadFragShader("graviton_formation_frag.glsl");
    
    setupPingPongFbo();
    // THE VBO HAS TO BE DRAWN AFTER FBO!
    setupVBO();
    
    // textures
    mParticleTexture1 = gl::Texture( loadImage( app::loadResource( "particle_white.png" ) ) );
    mParticleTexture2 = gl::Texture( loadImage( app::loadResource( "glitter.png" ) ) );
    mParticleTexture1.setWrap( GL_REPEAT, GL_REPEAT );
    mParticleTexture2.setWrap( GL_REPEAT, GL_REPEAT );
    
    mCameraController.setup(mApp, 0, CameraController::CAM_SPLINE);
    mAudioInputHandler.setup(false);
    
    reset();
}

void Graviton::setupPingPongFbo()
{
	std::vector<Surface32f> surfaces;
    // Position 2D texture array
    surfaces.push_back( generatePositionSurface() );
    //Velocity 2D texture array
    surfaces.push_back( generateVelocitySurface() );
    
    mParticlesFbo = PingPongFbo( surfaces );
}

Surface32f Graviton::generatePositionSurface()
{
    float r = mFormationRadius;
    
    Surface32f surface( kStep, kStep, true );
    Surface32f::Iter pixelIter = surface.getIter();
    while( pixelIter.line() )
    {
        while( pixelIter.pixel() )
        {
            double x = 0.0f;
            double y = 0.0f;
            double z = 0.0f;
            double mass = Rand::randFloat(0.1f, 1.0f);
            
            
            switch( mInitialFormation )
            {
                case FORMATION_SPHERE:
                {
                    double rho = Utils::randDouble() * (M_PI * 2.0);
                    double theta = Utils::randDouble() * (M_PI * 2.0);
                    
                    const float d = Rand::randFloat(10.0f, r);
                    x = d * cos(rho) * sin(theta);
                    y = d * sin(rho) * sin(theta);
                    z = d * cos(theta);
                }
                    break;
                    
                case FORMATION_SPHERE_SHELL:
                {
                    double rho = Utils::randDouble() * (M_PI * 2.0);
                    double theta = Utils::randDouble() * (M_PI * 2.0);
                    
                    x = r * cos(rho) * sin(theta);
                    y = r * sin(rho) * sin(theta);
                    z = r * cos(theta);
                }
                    break;
                    
                case FORMATION_CUBE:
                    x = r*(Rand::randFloat()-0.5f);
                    y = r*(Rand::randFloat()-0.5f);
                    z = r*(Rand::randFloat()-0.5f);
                    break;
                    
                default:
                    break;
            }

            // RGB = position
            // A = mass
            surface.setPixel(pixelIter.getPos(), ColorAf(x, y, z, mass) );
        }
    }
    
    return surface;
}

Surface32f Graviton::generateVelocitySurface()
{
    Surface32f surface( kStep, kStep, true );
    Surface32f::Iter pixelIter = surface.getIter();
    while( pixelIter.line() ) {
        while( pixelIter.pixel() ) {
            /* Initial particle velocities are
             passed in as R,G,B float values. */
            surface.setPixel( pixelIter.getPos(), ColorAf( 0.0f, 0.0f, 0.0f, 1.0f ) );
        }
    }
    
    return surface;
}

void Graviton::setupVBO()
{
    // A dummy VboMesh the same size as the texture to keep the vertices on the GPU
    int totalVertices = kStep * kStep;
    vector<Vec2f> texCoords;
    vector<uint32_t> indices;
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    layout.setStaticTexCoords2d();
    //layout.setStaticNormals();
    //layout.setDynamicColorsRGBA();
    mVboMesh = gl::VboMesh( totalVertices, totalVertices, layout, GL_POINTS);
    for( int x = 0; x < kStep; ++x ) {
        for( int y = 0; y < kStep; ++y ) {
            indices.push_back( x * kStep + y );
            texCoords.push_back( Vec2f( x/(float)kStep, y/(float)kStep ) );
        }
    }
    mVboMesh.bufferIndices( indices );
    mVboMesh.bufferTexCoords2d( 0, texCoords );
    mVboMesh.unbindBuffers();
}

void Graviton::setupDebugInterface()
{
    Scene::setupDebugInterface(); // add all interface params
    
}

void Graviton::setupInterface()
{
    mInterface->gui()->addSeparator();
    mInterface->gui()->addLabel("formation");
    vector<string> formationNames;
#define GRAVITON_FORMATION_ENTRY( nam, enm ) \
formationNames.push_back(nam);
    GRAVITON_FORMATION_TUPLE
#undef  GRAVITON_FORMATION_ENTRY
    mInterface->addEnum(CreateEnumParam( "Particle Formation", (int*)(&mInitialFormation) )
                        .maxValue(FORMATION_COUNT)
                        .oscReceiver(getName(), "pformation")
                        .isVertical(), formationNames);
    
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
    mInterface->addParam(CreateColorParam("color", &mColorScale, kMinColor, kMaxColor)
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam( "pointsize", &mPointSize )
                         .minValue(0.01f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "texturedpoints", &mUseImageForPoints ));
    
    mCameraController.setupInterface(mInterface, mName);
    mAudioInputHandler.setupInterface(mInterface, mName);
}

void Graviton::initParticles()
{
    gl::Texture::Format format;
    format.setInternalFormat( GL_RGBA32F_ARB );
    
	gl::Texture posTexture = gl::Texture( generatePositionSurface(), format );
    posTexture.setWrap( GL_REPEAT, GL_REPEAT );
    posTexture.setMinFilter( GL_NEAREST );
    posTexture.setMagFilter( GL_NEAREST );
    
    gl::Texture velTexture = gl::Texture( generateVelocitySurface(), format );
    velTexture.setWrap( GL_REPEAT, GL_REPEAT );
    velTexture.setMinFilter( GL_NEAREST );
    velTexture.setMagFilter( GL_NEAREST );
    
    posTexture.bind(3);
    velTexture.bind(2);
    
    mFormationShader.bind();
    mFormationShader.uniform( "positions", 3 );
    mFormationShader.uniform( "velocities", 2 );
    
    // draw to particle fbo
    gl::pushMatrices();
    gl::setMatricesWindow( mParticlesFbo.getSize(), false ); // false to prevent vertical flipping
    gl::setViewport( mParticlesFbo.getBounds() );
    
    mParticlesFbo.bindUpdate();
    gl::drawSolidRect(mParticlesFbo.getBounds());
    mParticlesFbo.unbindUpdate();
    
    mParticlesFbo.bindUpdate();
    gl::drawSolidRect(mParticlesFbo.getBounds());
    mParticlesFbo.unbindUpdate();
    
    gl::popMatrices();
    
    posTexture.unbind();
    velTexture.unbind();
    mFormationShader.unbind();
}

void Graviton::reset()
{
    resetGravityNodes();
    initParticles();
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
    Scene::update(dt);
    
    mAudioInputHandler.update(dt, mApp->getAudioInput(), mGain);
    
    if (mAudioGravity) {
        mGravity = MIN(mAudioInputHandler.getAverageVolumeHighFreq(), 1.0f);
    }

    // update particle system
    //computeAttractorPosition();
    updateGravityNodes(dt * 0.005f);
    
    gl::pushMatrices();
    gl::setMatricesWindow( mParticlesFbo.getSize(), false ); // false to prevent vertical flipping
    gl::setViewport( mParticlesFbo.getBounds() );
    
    mParticlesFbo.bindUpdate();
    
    mSimulationShader.bind();
    mSimulationShader.uniform( "positions", 0 );
    mSimulationShader.uniform( "velocities", 1 );
    mSimulationShader.uniform( "dt", (float)(dt * mTimeStep * 100.0f) );
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
    
    gl::drawSolidRect(mParticlesFbo.getBounds());
    mSimulationShader.unbind();
    
    mParticlesFbo.unbindUpdate();
    gl::popMatrices();
    
    mCameraController.update(dt);
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
    return mCameraController.getCamera();
}

#pragma mark - Render

void Graviton::draw()
{
    gl::pushMatrices();
    
    drawParticles();
    
    gl::popMatrices();
}

void Graviton::preRender() 
{
    gl::setMatrices( getCamera() );
    gl::setViewport( mApp->getViewportBounds() );

    gl::enable(GL_POINT_SPRITE);
    gl::enable(GL_PROGRAM_POINT_SIZE_EXT);
    glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
    // when false, point sprites just have one texture coordinate
    // when true, iterates over the point sprite texture...
    // TODO: understand what the difference is!
    // when it is on, the color param from the displacement_vert shaders cannot be applied to the frag color in displacement_frag, it always looks like a gradient between green/yellow instead... have on idea why
    // also don't really know what enabling this actually does so leaving it off
    // until i understand...
    //glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	
	if(mAdditiveBlending)
    {
        gl::enableAdditiveBlending();
	}
    else 
    {
		gl::enableAlphaBlending();
	}
    
    gl::disableDepthWrite();
}


void Graviton::drawParticles() 
{
    glPushAttrib( GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT );
    preRender();
    glEnable(GL_TEXTURE_2D);
    if(mUseImageForPoints)
    {
        mParticleTexture1.bind(2);
    }
    else
    {
        mParticleTexture2.bind(2);
    }
    
    if (mApp->getAudioInputHandler().hasTexture())
    {
        mApp->getAudioInputHandler().getFbo().bindTexture(3);
    }
    
    mParticlesFbo.bindTexture(0);
    mParticlesFbo.bindTexture(1);
    mRenderShader.bind();
    mRenderShader.uniform("displacementMap", 0 );
    mRenderShader.uniform("velocityMap", 1);
    mRenderShader.uniform("pointSpriteTex", 2);
    mRenderShader.uniform("intensityMap", 3);
    mRenderShader.uniform("screenWidth", (float)mApp->getViewportWidth());
    mRenderShader.uniform("spriteWidth", mPointSize);
    mRenderShader.uniform("MV", getCamera().getModelViewMatrix());
    mRenderShader.uniform("P", getCamera().getProjectionMatrix());
    mRenderShader.uniform("colorScale", mColorScale);
    mRenderShader.uniform("gain", mGain);
    
    gl::draw( mVboMesh );
    mRenderShader.unbind();
    mParticlesFbo.unbindTexture();
    
    if(mUseImageForPoints)
    {
        mParticleTexture1.unbind();
    }
    else
    {
        mParticleTexture2.unbind();
    }
    if (mApp->getAudioInputHandler().hasTexture())
    {
        mApp->getAudioInputHandler().getFbo().unbindTexture();
    }
    glPopAttrib();
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
    Rectf preview( windowSize.x - (size+paddingX), windowSize.y - (size+paddingY), windowSize.x-paddingX, windowSize.y - paddingY );
    
    gl::draw( mParticlesFbo.getTexture(0), preview );
    
    Rectf preview2 = preview - Vec2f(size+paddingX, 0.0f);
    gl::draw( mParticlesFbo.getTexture(1), preview2 );
    
    glPopAttrib();
    gl::popMatrices();
    
    
    //drawCamSpline();
}
