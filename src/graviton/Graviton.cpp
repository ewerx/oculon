//
// Graviton.cpp
// Oculon
//
// Created by Ehsan on 11-11-26.
// Copyright 2011 ewerx. All rights reserved.
//


#include "Graviton.h"
#include "Resources.h"
#include "OculonApp.h"
#include "cinder/Rand.h"
#include "cinder/Easing.h"
#include "Utils.h"
#include "Interface.h"
#include <boost/foreach.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;


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
    
    mConstrainParticles = false;
    
    mInitialFormation = FORMATION_SPHERE;
    mFormationRadius = 80.0f;
    
    mAdditiveBlending = true;
    mUseImageForPoints = true;
    mPointSize = 0.6f;
    mColorScale = ColorAf( 0.5f, 0.5f, 0.6f, 0.5f );
    
    mDamping = 0.0f;
    mGravity = 0.05f;
    mEps = 0.0f;//0.001f;//mFormationRadius * 0.5f;
    mConstraintSphereRadius = 0.0f;
    mNodeSpeed = 1.0f;
    
    mNumNodes = 0;
    mGravityNodeFormation = NODE_FORMATION_STATIC;
    
    // camera
    mCamRadius = mFormationRadius * 3.0f;
    mCamAngle = 0.0f;
    mCamMaxDistance = mFormationRadius * 3.0f;
    mCamLateralPosition = -mCamMaxDistance;
    mCamTarget = Vec3f::zero();
    mCamTurnRate = 0.25f;
    mCamTranslateRate = 1.0f;
    mCamType = CAM_SPLINE;
    
    // shaders
    mParticlesShader = loadFragShader("graviton_particle_frag.glsl" );
    mDisplacementShader = loadVertAndFragShaders("graviton_displacement_vert.glsl",  "graviton_displacement_frag.glsl");
    mFormationShader = loadFragShader("graviton_formation_frag.glsl");
    
    setupPingPongFbo();
    // THE VBO HAS TO BE DRAWN AFTER FBO!
    setupVBO();
    
    // textures
    mParticleTexture1 = gl::Texture( loadImage( app::loadResource( "particle_white.png" ) ) );
    mParticleTexture2 = gl::Texture( loadImage( app::loadResource( "glitter.png" ) ) );
    mParticleTexture1.setWrap( GL_REPEAT, GL_REPEAT );
    mParticleTexture2.setWrap( GL_REPEAT, GL_REPEAT );
    
    // renderers
    mMotionBlurRenderer.setup( mApp->getViewportSize(), boost::bind( &Graviton::drawParticles, this ) );
    
    // random spline
    setupCameraSpline();
    // camera
    mCamAngle = 0.0f;
    mCamLateralPosition = -mCamMaxDistance;
    mCamTarget = Vec3f::zero();
    mCam.setFov(60.0f);
    mCam.setAspectRatio(mApp->getViewportAspectRatio());
    
    reset();
}

void Graviton::setupPingPongFbo()
{
    // TODO: Test with more than 2 texture attachments -- what for?
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
    layout.setStaticNormals();
    //layout.setDynamicColorsRGBA();
    glPointSize(1.0f);
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
    
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("display");
    mInterface->addParam(CreateColorParam("color", &mColorScale, kMinColor, kMaxColor)
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam( "pointsize", &mPointSize )
                         .minValue(0.01f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "texturedpoints", &mUseImageForPoints ));
    
    mInterface->gui()->addColumn();
    vector<string> camTypeNames;
#define GRAVITON_CAMTYPE_ENTRY( nam, enm ) \
camTypeNames.push_back(nam);
    GRAVITON_CAMTYPE_TUPLE
#undef  GRAVITON_CAMTYPE_ENTRY
    mInterface->addEnum(CreateEnumParam( "camera", (int*)(&mCamType) )
                        .maxValue(CAM_COUNT)
                        .oscReceiver(getName())
                        .isVertical(), camTypeNames);
    mInterface->gui()->addLabel("spline cam");
    mInterface->addParam(CreateFloatParam( "radius", &mCamRadius )
                         .minValue(1.0f)
                         .maxValue(500.f)
                         .oscReceiver(getName(), "camradius"));
    mInterface->addParam(CreateFloatParam( "distance", &mCamMaxDistance )
                         .minValue(0.0f)
                         .maxValue(500.f));
    mInterface->addParam(CreateFloatParam( "turn speed", &mCamTurnRate )
                         .minValue(0.0f)
                         .maxValue(5.0f)
                         .oscReceiver(getName(), "camturn"));
    mInterface->addParam(CreateFloatParam( "slide speed", &mCamTranslateRate )
                         .minValue(0.0f)
                         .maxValue(5.0f)
                         .oscReceiver(getName(), "camspeed"));
    mInterface->addButton(CreateTriggerParam("reset spline", NULL)
                          .oscReceiver(mName,"resetspline"))->registerCallback( this, &Graviton::setupCameraSpline );
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
    
//    setupPingPongFbo();
//    setupVBO();
    
    
//    mNumParticles = kNumParticles;
//    
//    const double r = mFormationRadius;
//    
//    for( size_t i = 0; i < kNumParticles; ++i )
//    {
//        if( (NODE_FORMATION_NONE != mGravityNodeFormation) && i < mNumNodes)
//        {
//            mPosAndMass[i].x = mGravityNodes[i].mPos.x;
//            mPosAndMass[i].y = mGravityNodes[i].mPos.y;
//            mPosAndMass[i].z = mGravityNodes[i].mPos.z;
//            mPosAndMass[i].w = 1.0f;
//            
//            mVel[i].x = mGravityNodes[i].mVel.x;
//            mVel[i].x = mGravityNodes[i].mVel.y;
//            mVel[i].x = mGravityNodes[i].mVel.z;
//            mVel[i].w = mGravityNodes[i].mMass;
//            
//            mColor[i].x = 1.0f;
//            mColor[i].y = 1.0f;
//            mColor[i].z = 1.0f;
//            mColor[i].w = 1.0f;
//        }
//        else
//        {
//            double x = 0.0f;
//            double y = 0.0f;
//            double z = 0.0f;
//            
//            double vx = 0.0f;
//            double vy = 0.0f;
//            double vz = 0.0f;
//            
//            double rho = 0.0f;
//            double theta = 0.0f;
//            
//            const float maxMass = 50.0f;
//            float mass = Rand::randFloat(1.0f,maxMass);
//            
//            switch( mInitialFormation )
//            {
//                case FORMATION_SPHERE:
//                {
//                    rho = Utils::randDouble() * (M_PI * 2.0);
//                    theta = Utils::randDouble() * (M_PI * 2.0);
//                    
//                    const float d = Rand::randFloat(10.0f, r);
//                    x = d * cos(rho) * sin(theta);
//                    y = d * sin(rho) * sin(theta);
//                    z = d * cos(theta);
//                }
//                    break;
//                    
//                case FORMATION_SPHERE_SHELL:
//                {
//                    rho = Utils::randDouble() * (M_PI * 2.0);
//                    theta = Utils::randDouble() * (M_PI * 2.0);
//                    
//                    x = r * cos(rho) * sin(theta);
//                    y = r * sin(rho) * sin(theta);
//                    z = r * cos(theta);
//                }
//                    break;
//                    
//                case FORMATION_DISC:
//                {
//                    rho = r * Utils::randDouble();//pow(Utils::randDouble(), 0.75);
//                    theta = Utils::randDouble() * (M_PI * 2.0);
//                    theta = (0.5 * cos(2.0 * theta) + theta - 1e-2 * rho);
//                    
//                    const float thickness = 1.0f;
//                    
//                    x = rho * cos(theta);
//                    y = rho * sin(theta);
//                    z = thickness * 2.0 * Utils::randDouble() - 1.0;
//                    
//                    const double a = 1.0e0 * (rho <= 1e-1 ? 0.0 : rho);
//                    vx = -a * sin(theta);
//                    vy = a * cos(theta);
//                    vz = 0.0f;
//                }
//                    break;
//                    
//                case FORMATION_GALAXY:
//                {
//                    rho = r * pow(Utils::randDouble(), 0.75);
//                    theta = Utils::randDouble() * (M_PI * 2.0);
//                    theta = (0.5 * cos(2.0 * theta) + theta - 1e-2 * rho);
//                    
//                    x = rho * cos(theta);
//                    y = rho * sin(theta);
//                    
//                    const float dist = sqrt(x*x + y*y);
//                    const float maxThickness = r / 8.0f;
//                    const float coreDistanceRatio = EaseInOutQuad()(1.0f - dist / r);
//                    const float thickness =  maxThickness * coreDistanceRatio;
//                    
//                    z = thickness * (2.0 * Utils::randDouble() - 1.0);
//                    
//                    const double a = 1.0e0 * (rho <= 1e-1 ? 0.0 : rho);
//                    vx = -a * sin(theta);
//                    vy = a * cos(theta);
//                    vz = 0.0f;
//                    
//                    mass = maxMass * coreDistanceRatio;
//                }
//                    break;
//                    
//                default:
//                    break;
//            }
//            
//            // pos
//            mPosAndMass[i].x = x;
//            mPosAndMass[i].y = y;
//            mPosAndMass[i].z = z;
//            mPosAndMass[i].w = 1.0f; //scale??
//            
//            // vel
//            
//            mVel[i].x = vx;
//            mVel[i].y = vy;
//            mVel[i].z = vz;
//            mVel[i].w = mass;
//            
//            // color
//            mColor[i].x = 1.0f;
//            mColor[i].y = 1.0f;
//            mColor[i].z = 1.0f;
//            mColor[i].w = 1.0f;
//        }
//    }
}
                              
bool Graviton::resetGravityNodes()
{
    mGravityNodes.clear();
    
    const float r = mConstraintSphereRadius > 0.0f ? mConstraintSphereRadius : mFormationRadius;
    
    switch( mGravityNodeFormation )
    {
        case NODE_FORMATION_SPIN:
        {
            mNumNodes = 1;
            
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
        
        case NODE_FORMATION_RANDOM:
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

void Graviton::reset()
{
    resetGravityNodes();
    initParticles();
}

void Graviton::resize()
{
    Scene::resize();
    mMotionBlurRenderer.resize(mApp->getViewportSize());
}

#pragma mark - Update

void Graviton::update(double dt)
{
    updateAudioResponse();

    // update particle system
    //computeAttractorPosition();
    updateGravityNodes(dt * 0.005f);
    
    gl::pushMatrices();
    gl::setMatricesWindow( mParticlesFbo.getSize(), false ); // false to prevent vertical flipping
    gl::setViewport( mParticlesFbo.getBounds() );
    
    mParticlesFbo.bindUpdate();
    
    mParticlesShader.bind();
    mParticlesShader.uniform( "positions", 0 );
    mParticlesShader.uniform( "velocities", 1 );
    mParticlesShader.uniform( "dt", (float)(dt * mTimeStep * 100.f) );
    mParticlesShader.uniform( "eps", mEps );
    mParticlesShader.uniform( "damping", mDamping );
    mParticlesShader.uniform( "gravity", mGravity );
    mParticlesShader.uniform( "containerradius", mConstraintSphereRadius );
    mParticlesShader.uniform( "attractorPos1", mGravityNodes[0].mPos);
    mParticlesShader.uniform( "attractorPos2", mGravityNodes[1].mPos);
    mParticlesShader.uniform( "attractorPos3", mGravityNodes[2].mPos);
    
    gl::drawSolidRect(mParticlesFbo.getBounds());
    mParticlesShader.unbind();
    
    mParticlesFbo.unbindUpdate();
    gl::popMatrices();
    
    updateCamera(dt);
    
    Scene::update(dt);
}

void Graviton::updateGravityNodes(const double dt)
{
    for (int i = 0; i < mGravityNodes.size(); ++i)
    {
        switch (mGravityNodeFormation) {
            case NODE_FORMATION_RANDOM:
            {
                mGravityNodes[i].mPos += mGravityNodes[i].mVel * dt * 1000.f * mNodeSpeed;
                float distance = mGravityNodes[i].mPos.length();
                
                if (i == 0 && ((mConstraintSphereRadius > 0.0f && distance > mConstraintSphereRadius) || distance > 1000.f))
                {
                    mGravityNodes[i].mPos = mGravityNodes[i].mPos.normalized() * mConstraintSphereRadius;
                    mGravityNodes[i].mVel.x *= Rand::randFloat(-1.25f, -0.75f);
                    mGravityNodes[i].mVel.y *= Rand::randFloat(-1.25f, -0.75f);
                    mGravityNodes[i].mVel.z *= Rand::randFloat(-1.25f, -0.75f);
                }
            }
            break;
            
            case NODE_FORMATION_SPIN:
            {
                if (mConstraintSphereRadius > 0.0f) {
                    mGravityNodes[i].mVel.x += mGravityNodes[i].mVel.z * dt;
                    mGravityNodes[i].mVel.y += mGravityNodes[i].mVel.z * dt;
                    float theta = mGravityNodes[i].mVel.x;
                    float rho = mGravityNodes[i].mVel.y;
                    mGravityNodes[i].mPos = Vec3f(mConstraintSphereRadius*cos(rho)*cos(theta), mConstraintSphereRadius*sin(theta), mConstraintSphereRadius*sin(rho)*cos(theta));
                }
            }
            break;
            
            default:
            break;
        }
        
        if (i==0) {
            timeline().apply( &mCamTarget, mGravityNodes[i].mPos, 0.2f, EaseInQuad() );
        }
    }
}

#pragma mark - Input

bool Graviton::handleKeyDown(const KeyEvent& keyEvent)
{
    bool handled = true;
    
    switch (keyEvent.getChar()) 
    {
        case ' ':
            reset();
            break;
        default:
            handled = false;
            break;
    }
    
    return handled;
}

void Graviton::handleMouseDown( const MouseEvent& event )
{
	mIsMousePressed = true;
	mMousePos.x = event.getPos().x;
    mMousePos.y = event.getPos().y;
}

void Graviton::handleMouseUp( const MouseEvent& event )
{
	mIsMousePressed = false;
}

void Graviton::handleMouseDrag( const MouseEvent& event )
{
	mMousePos.x = event.getPos().x;
    mMousePos.y = event.getPos().y;
}

//
//
//
void Graviton::updateAudioResponse()
{
}

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

void Graviton::updateCamera(const double dt)
{
    switch( mCamType )
    {
        case CAM_SPLINE:
        {
            mCamSplineValue += dt * (mCamTranslateRate*0.01f);
            Vec3f pos = mCamSpline.getPosition( mCamSplineValue );
            Vec3f delta = pos - mCamLastPos;
            Vec3f up = delta.cross(pos);
            //up.normalize();
            mCam.lookAt( pos, mCamTarget, up );
            mCamLastPos = pos;
        }
            break;
            
        case CAM_SPIRAL:
        {
            //TODO: use quaternion
            //Quatf incQuat( normal, rotation );
            //mQuat *= incQuat;
            //mQuat.normalize();
            mCamAngle += dt * mCamTurnRate;
            mCamLateralPosition += dt * mCamTranslateRate;
            if( ( (mCamLateralPosition > mCamMaxDistance) && mCamTranslateRate > 0.0f ) ||
               ( (mCamLateralPosition < -mCamMaxDistance) && mCamTranslateRate < 0.0f ) )
            {
                mCamTranslateRate = -mCamTranslateRate;
            }
            
            Vec3f pos(mCamRadius * cos(mCamAngle),
                      mCamRadius * sin(mCamAngle),
                      mCamLateralPosition );
            
            Vec3f up( pos.x, pos.y, 0.0f );
            up.normalize();
            mCam.lookAt( pos, mCamTarget, up );
        }
            break;
            
        default:
            break;
    }
    
}

const Camera& Graviton::getCamera()
{
    switch( mCamType )
    {
        case CAM_SPLINE:
        case CAM_SPIRAL:
            return( mCam );
            
        case CAM_ORBITER:
        {
            Scene* orbiterScene = mApp->getScene("orbiter");
            
            if( orbiterScene && orbiterScene->isRunning() )
            {
                return( orbiterScene->getCamera() );
            }
            else
            {
                return( mApp->getMayaCam() );
            }
        }
            break;
            
        default:
            return mApp->getMayaCam();
    }
}

bool Graviton::setupCameraSpline()
{
    vector<Vec3f> points;
	int numPoints = 4 + ( Rand::randInt(4) );
	for( int p = 0; p < numPoints; ++p )
    {
		points.push_back( Vec3f( Rand::randFloat(-mCamRadius/2.0f, mCamRadius/2.0f), Rand::randFloat(-mCamRadius/2.0f, mCamRadius/2.0f), Rand::randFloat(-mCamRadius/2.0f, mCamRadius/2.0f) ) );
    }
	mCamSpline = BSpline3f( points, 3, true, false );
    
	mCamSplineValue = 0.0f;
	mCamRotation = Quatf::identity();
	mCamLastPos = mCamSpline.getPosition( 0 );
    
    return false;
}

void Graviton::drawCamSpline()
{
    gl::pushMatrices();
    gl::setMatrices( ( mCamType == CAM_MANUAL ) ? mApp->getMayaCam() : mCam );
	const int numSegments = 100;
	gl::color( ColorA( 0.2f, 0.85f, 0.8f, 0.85f ) );
	glLineWidth( 2.0f );
	gl::begin( GL_LINE_STRIP );
	for( int s = 0; s <= numSegments; ++s ) 
    {
		float t = s / (float)numSegments;
		gl::vertex( mCamSpline.getPosition( t ) );
	}
	gl::end();
    gl::popMatrices();
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
    mDisplacementShader.bind();
    mDisplacementShader.uniform("displacementMap", 0 );
    mDisplacementShader.uniform("velocityMap", 1);
    mDisplacementShader.uniform("pointSpriteTex", 2);
    mDisplacementShader.uniform("intensityMap", 3);
    mDisplacementShader.uniform("screenWidth", (float)mApp->getViewportWidth());
    mDisplacementShader.uniform("spriteWidth", mPointSize);
    mDisplacementShader.uniform("MV", getCamera().getModelViewMatrix());
    mDisplacementShader.uniform("P", getCamera().getProjectionMatrix());
    mDisplacementShader.uniform("colorScale", mColorScale);
    mDisplacementShader.uniform("gain", mGain);
    
    gl::draw( mVboMesh );
    mDisplacementShader.unbind();
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
