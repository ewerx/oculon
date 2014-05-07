//
//  Lines.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 11/21/2013.
//
//


#include "cinder/Perlin.h"
#include "cinder/Rand.h"

#include "Lines.h"
#include "OculonApp.h"
#include "Interface.h"

using namespace ci;
using namespace ci::app;
using namespace std;


#pragma mark - Construction

Lines::Lines()
: Scene("lines")
{
}

Lines::~Lines()
{
}

#pragma mark - Setup

void Lines::setup()
{
    Scene::setup();
    
    mReset = false;
    
    // params
    mTimeStep = 0.1f;
    mFormationStep = 1.0f;
    mFormationAnimSelector.mDuration = 0.75f;
    mMotion = MOTION_NOISE;
    
    mAudioTime = false;
    
    // simulation
    mSimulationShader = loadVertAndFragShaders("lines_simulation_vert.glsl", "lines_simulation_frag.glsl");
    
    const int bufSize = 128;
    setupParticles(bufSize);
    
    // rendering
    mParticleController.addRenderer( new LinesRenderer() );
    mParticleController.addRenderer( new GravitonRenderer() );
    mParticleController.addRenderer( new DustRenderer() );
    
    mDynamicTexture.setup(bufSize, bufSize);
    
    mCameraController.setup(mApp, CameraController::CAM_MANUAL|CameraController::CAM_SPLINE, CameraController::CAM_MANUAL);
    mApp->setCamera(Vec3f(480.0f, 0.0f, 0.0f), Vec3f(-1.0f, 0.0f, 0.0f), Vec3f(0.0f,1.0f,0.0f));
    
    // audio
    mAudioInputHandler.setup(true);
}

void Lines::setupParticles(const int bufSize)
{
    int numParticles = bufSize*bufSize;
    console() << "[lines] initializing " << numParticles << " particles, hang on!" << std::endl;
    
    mParticleController.setup(bufSize);
    
    vector<Vec4f> randomPositions;
    vector<Vec4f> straightPositions;
    vector<Vec4f> velocities;
    vector<Vec4f> data;
    
    // random
    for (int i = 0; i < numParticles; ++i)
    {
        // position + mass
        float x = Rand::randFloat(-1.0f,1.0f);
        float y = Rand::randFloat(-1.0f,1.0f);
        float z = Rand::randFloat(-1.0f,1.0f);
        float mass = Rand::randFloat(0.01f,1.0f);
        randomPositions.push_back(Vec4f(x,y,z,mass));
        
        // velocity + age
        float vx = Rand::randFloat(-.005f,.005f);
        float vy = Rand::randFloat(-.005f,.005f);
        float vz = Rand::randFloat(-.005f,.005f);
        float age = Rand::randFloat(.007f,0.9f);
        velocities.push_back(Vec4f(vx,vy,vz,age));
        
        // extra info
        float decay = Rand::randFloat(.01f,10.00f);
        data.push_back(Vec4f(x,y,z,decay));
    }
    mParticleController.addFormation(new ParticleFormation("random", bufSize, randomPositions, velocities, data));
    
    // straight lines
    bool pair = false;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    
    for (int i = 0; i < numParticles; ++i)
    {
        if (pair)
        {
            int axis = Rand::randInt(3); // x=0,y=1,z=2
            switch(axis)
            {
                case 0:
                    x = -x;
                    break;
                case 1:
                    y = -y;
                    break;
                case 2:
                    z = -z;
                    break;
                default:
                    break;
            }
        }
        else
        {
            x = Rand::randFloat(-1.0f,1.0f);
            y = Rand::randFloat(-1.0f,1.0f);
            z = Rand::randFloat(-1.0f,1.0f);
        }
        
        float mass = Rand::randFloat(0.01f,1.0f);
        
        // position + mass
        straightPositions.push_back(Vec4f(x,y,z,mass));
        
        pair = !pair;
    }
    
    mParticleController.addFormation(new ParticleFormation("straight", bufSize, straightPositions, velocities, data));
    
    // TODO: refactor into a ParticleController::completeSetup method... is there a better way? first update?
    mParticleController.resetToFormation(0);
}

void Lines::reset()
{
    mReset = true;
    
    mApp->setCamera(Vec3f(580.0f, 0.0f, 0.0f), Vec3f(-1.0f, 0.0f, 0.0f), Vec3f(0.0f,1.0f,0.0f));
}

#pragma mark - Interface

void Lines::setupInterface()
{
    mInterface->addParam(CreateFloatParam( "timestep", &mTimeStep )
                         .minValue(0.0001f)
                         .maxValue(0.1f)
                         .oscReceiver(mName));
    
    mInterface->gui()->addColumn();
    vector<string> motionNames;
#define MOTION_ENTRY( nam, enm ) \
    motionNames.push_back(nam);
    MOTION_TUPLE
#undef  MOTION_ENTRY
    mInterface->addEnum(CreateEnumParam( "behavior", (int*)(&mMotion) )
                        .maxValue(MOTION_COUNT)
                        .isVertical()
                        .oscReceiver(mName)
                        .sendFeedback(), motionNames);
    
    mInterface->addParam(CreateFloatParam( "formation_step", mFormationStep.ptr() ));
    mFormationAnimSelector.setupInterface(mInterface, mName);
    
    mDynamicTexture.setupInterface(mInterface, mName);
    
    mInterface->gui()->addColumn();
    mParticleController.setupInterface(mInterface, mName);
    mParticleController.getFormationChangedSignal().connect( bind(&Lines::takeFormation, this) );
    
    mCameraController.setupInterface(mInterface, mName);
    mAudioInputHandler.setupInterface(mInterface, mName);
}

#pragma mark - Callbacks

void Lines::takeFormation()
{
    mFormationStep = 0.0f;
    timeline().apply( &mFormationStep, 1.0f, mFormationAnimSelector.mDuration,mFormationAnimSelector.getEaseFunction() );
    mFormationStep = 0.0f;
}

#pragma mark - Update

void Lines::update(double dt)
{
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    mCameraController.update(dt);
    
    gl::disableAlphaBlending();
    
    gl::disableAlphaBlending();
	gl::disableDepthRead();
	gl::disableDepthWrite();
    
    mDynamicTexture.update(dt);
    
    // TODO: refactor
    PingPongFbo& mParticlesFbo = mParticleController.getParticleFbo();
    
    gl::pushMatrices();
    gl::setMatricesWindow( mParticlesFbo.getSize(), false ); // false to prevent vertical flipping
    gl::setViewport( mParticlesFbo.getBounds() );
    
    mParticlesFbo.bindUpdate();
    
    mParticleController.getFormation().getPositionTex().bind(3);
    mParticleController.getFormation().getVelocityTex().bind(4);
    
    mDynamicTexture.bindTexture(5);
    
    float simdt = (float)(dt*mTimeStep);
    if (mAudioTime) simdt *= (1.0 - mAudioInputHandler.getAverageVolumeLowFreq());
    mSimulationShader.bind();
    mSimulationShader.uniform( "positions", 0 );
    mSimulationShader.uniform( "velocities", 1 );
    mSimulationShader.uniform( "information", 2);
    mSimulationShader.uniform( "oPositions", 3);
	mSimulationShader.uniform( "oVelocities", 4);
  	mSimulationShader.uniform( "noiseTex", 5);
    mSimulationShader.uniform( "dt", (float)dt );
    mSimulationShader.uniform( "reset", mReset );
    mSimulationShader.uniform( "formationStep", mFormationStep );
    mSimulationShader.uniform( "motion", mMotion );
    mSimulationShader.uniform( "containmentSize", 3.0f );
    
    gl::drawSolidRect(mParticlesFbo.getBounds());
    
    mSimulationShader.unbind();
    
    mDynamicTexture.unbindTexture();
    
    mParticleController.getFormation().getPositionTex().unbind();
    mParticleController.getFormation().getVelocityTex().unbind();
    
    mParticlesFbo.unbindUpdate();
    gl::popMatrices();
    
    mReset = false;
    
    Scene::update(dt);
}

#pragma mark - Draw

const Camera& Lines::getCamera()
{
    return mCameraController.getCamera();
}

void Lines::draw()
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

void Lines::drawDebug()
{
    Scene::drawDebug();
    
    gl::pushMatrices();
    glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
    gl::enable( GL_TEXTURE_2D );
    const Vec2f& windowSize( getWindowSize() );
    gl::setMatricesWindow( windowSize );
    
    const float size = 80.0f;
    const float paddingX = 20.0f;
    const float paddingY = 240.0f;
    
    //HACK
    PingPongFbo& mParticlesFbo = mParticleController.getParticleFbo();
    
    Rectf preview( windowSize.x - (size+paddingX), windowSize.y - (size+paddingY), windowSize.x-paddingX, windowSize.y - paddingY );
    gl::draw( mParticlesFbo.getTexture(0), preview );
    
    Rectf preview2 = preview - Vec2f(size+paddingX, 0.0f);
    gl::draw( mParticlesFbo.getTexture(1), preview2 );
    
    Rectf preview3 = preview2 - Vec2f(size+paddingX, 0.0f);
    gl::draw(mDynamicTexture.getTexture(), preview3);
    
    glPopAttrib();
    gl::popMatrices();
}
