//
//  Flock2.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 10/10/2015.
//
//


#include "cinder/Perlin.h"
#include "cinder/Rand.h"

#include "Flock2.h"
#include "OculonApp.h"
#include "Interface.h"
#include "SpinCam.h"
#include "SplineCam.h"
#include "OtherSceneCam.h"

using namespace ci;
using namespace ci::app;
using namespace std;


#pragma mark - Construction

Flock2::Flock2()
: Scene("flock2")
{
}

Flock2::~Flock2()
{
}

#pragma mark - Setup

void Flock2::setup()
{
    Scene::setup();
    
    mReset = false;
    
    // params
    
    
    // Predators
    mPredatorSimShader = loadVertAndFragShaders("lines_simulation_vert.glsl", "lines_Predatorsim_frag.glsl");
    mPredatorBufSize = 48;
    setupPredators(mPredatorBufSize); // 48*48 Predators
    
    // simulation
    mSimulationShader = loadVertAndFragShaders("lines_simulation_vert.glsl", "lines_simulation_frag.glsl");
    
    const int bufSize = 64;
    setupParticles(bufSize); // 64*64 particles
    
    // rendering
//    mParticleController.addRenderer( new FlockRenderer() );
    mParticleController.addRenderer( new GravitonRenderer() );
//    mParticleController.addRenderer( new DustRenderer() );
    
    mCameraController.setup(mApp);
    mCameraController.addCamera( new SpinCam(mApp->getViewportAspectRatio()) );
    mCameraController.addCamera( new SplineCam(mApp->getViewportAspectRatio()) );
    mCameraController.addCamera( new OtherSceneCam(mApp, "graviton") );
    mCameraController.addCamera( new OtherSceneCam(mApp, "parsec") );
    mCameraController.setCamIndex(1);
    
    
    mApp->setCamera(Vec3f(100.0f, 0.0f, 0.0f), Vec3f(-1.0f, 0.0f, 0.0f), Vec3f(0.0f,1.0f,0.0f));
    
    // audio
    mAudioInputHandler.setup(true);
    
    mTimeController.setTimeScale(0.01f);
}

void Flock2::setupParticles(const int bufSize)
{
    int numParticles = bufSize*bufSize;
    console() << "[lines] initializing " << (numParticles/2) << " lines..." << std::endl;
    
    mParticleController.setup(bufSize);
    
    vector<Vec4f> positions;
    vector<Vec4f> velocities;
    vector<Vec4f> data;
    
    const float r = 100.0f;
    
    // random
    {
        for (int i = 0; i < numParticles; ++i)
        {
//            int nodeIndex = Rand::randInt(mNodePositions.size());//Rand::randInt(mNodeBufSize*mNodeBufSize);
            
            // position + mass
            float x = r * Rand::randFloat(-1.0f,1.0f);
            float y = r * Rand::randFloat(-1.0f,1.0f);
            float z = r * Rand::randFloat(-1.0f,1.0f);
            float mass = Rand::randFloat(0.01f,1.0f);
            positions.push_back(Vec4f(x,y,z,mass));
            
            // velocity + age
            float vx = Rand::randFloat(-.005f,.005f);
            float vy = Rand::randFloat(-.005f,.005f);
            float vz = Rand::randFloat(-.005f,.005f);
            float age = Rand::randFloat(.007f,0.9f);
            velocities.push_back(Vec4f(vx,vy,vz,age));
            
            // extra info
//            float decay = Rand::randFloat(.01f,10.00f);
//            data.push_back(Vec4f(x,y,z,(float)nodeIndex));
        }
        mParticleController.addFormation(new ParticleFormation("random", bufSize, positions, velocities, data));
        positions.clear();
        data.clear();
    }
    

    // TODO: refactor into a ParticleController::completeSetup method... is there a better way? first update?
    mParticleController.resetToFormation(0);
}

//void Flock2::addParsecFormation( )

void Flock2::reset()
{
    mReset = true;
    
    mApp->setCamera(Vec3f(580.0f, 0.0f, 0.0f), Vec3f(-1.0f, 0.0f, 0.0f), Vec3f(0.0f,1.0f,0.0f));
}

#pragma mark - Interface

void Flock2::setupInterface()
{
    mTimeController.setupInterface(mInterface, getName(), 1, 18);
    
    mInterface->gui()->addColumn();

    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("Predators");
    
    mInterface->gui()->addColumn();
    mParticleController.setupInterface(mInterface, getName());
    
    mCameraController.setupInterface(mInterface, getName());
    mAudioInputHandler.setupInterface(mInterface, getName(), 1, 19, 2, 19);
}

#pragma mark - Update

void Flock2::update(double dt)
{
    mTimeController.update(dt);
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    mCameraController.update(dt);
    
    gl::disableAlphaBlending();
	gl::disableDepthRead();
	gl::disableDepthWrite();
    
    updatePredators(dt);
    
    updateParticles(dt);
    
    mReset = false;
    
    Scene::update(dt);
}

void Flock2::updateParticles(double dt)
{
    // TODO: refactor
    PingPongFbo& fbo = mParticleController.getParticleFbo();
    
    gl::pushMatrices();
    gl::setMatricesWindow( fbo.getSize(), false ); // false to prevent vertical flipping
    gl::setViewport( fbo.getBounds() );
    
    fbo.bindUpdate();
    
    mParticleController.getFormation().getPositionTex().bind(3);
    mParticleController.getFormation().getVelocityTex().bind(4);
    
    if (mAudioInputHandler.hasTexture())
    {
        mAudioInputHandler.getFbo().bindTexture(7);
    }
    
    // bind predator position tex
    mPredatorController.getParticleFbo().bindTexture(6, 0);
    
    float simdt = mTimeController.getDelta();
    mSimulationShader.bind();
    mSimulationShader.uniform( "positions", 0 );
    mSimulationShader.uniform( "velocities", 1 );
    mSimulationShader.uniform( "information", 2);
    mSimulationShader.uniform( "oPositions", 3);
	mSimulationShader.uniform( "oVelocities", 4);
  	mSimulationShader.uniform( "noiseTex", 5);
    mSimulationShader.uniform( "audioData", 7);
  	mSimulationShader.uniform( "nodePosTex", 6);
    mSimulationShader.uniform( "nodeBufSize", (float)mPredatorBufSize);
    mSimulationShader.uniform( "gain", mAudioInputHandler.getGain());
    mSimulationShader.uniform( "dt", (float)simdt );
    mSimulationShader.uniform( "reset", mReset );
    mSimulationShader.uniform( "startAnim", mParticleController.isStartingAnim() );
    mSimulationShader.uniform( "formationStep", mParticleController.getFormationStep() );
    
    gl::drawSolidRect(fbo.getBounds());
    
    mSimulationShader.unbind();
    
    mPredatorController.getFormation().getPositionTex().unbind();
    
    mParticleController.getFormation().getPositionTex().unbind();
    mParticleController.getFormation().getVelocityTex().unbind();
    
    fbo.unbindUpdate();
    gl::popMatrices();
}

#pragma mark - Draw

const Camera& Flock2::getCamera()
{
    return mCameraController.getCamera();
}

void Flock2::draw()
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

void Flock2::drawDebug()
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
    PingPongFbo& fbo = mParticleController.getParticleFbo();
    
    Rectf preview( windowSize.x - (size+paddingX), windowSize.y - (size+paddingY), windowSize.x-paddingX, windowSize.y - paddingY );
    gl::draw( fbo.getTexture(0), preview );
    
    Rectf preview2 = preview - Vec2f(size+paddingX, 0.0f);
    gl::draw( fbo.getTexture(1), preview2 );
    
//    Rectf preview3 = preview2 - Vec2f(size+paddingX, 0.0f);
    
    glPopAttrib();
    gl::popMatrices();
}

#pragma mark - Predators

void Flock2::setupPredators(const int bufSize)
{
    int numPredators = bufSize*bufSize;
    console() << "[lines] initializing " << numPredators << " Predators..." << std::endl;
    
    mPredatorController.setup(bufSize);
    
    vector<Vec4f> positions;
    vector<Vec4f> velocities;
    vector<Vec4f> data;
    
    const float r = 100.0f;
    
    // random
    {
        for (int i = 0; i < numPredators; ++i)
        {
            float rho = Rand::randFloat() * (M_PI * 2.0);
            float theta = Rand::randFloat() * (M_PI * 2.0);
            
            float x = r * cos(rho) * sin(theta);
            float y = r * sin(rho) * sin(theta);
            float z = r * cos(theta);
            
            float mass = Rand::randFloat(0.01f,1.0f);
            positions.push_back(Vec4f(x,y,z,mass));
            
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
        mPredatorController.addFormation(new ParticleFormation("random", bufSize, positions, velocities, data));
    }
    
    {
        positions.clear();
        
        for (int i = 0; i < numPredators; ++i)
        {
            float rho = Rand::randFloat() * (M_PI * 2.0);
            float theta = Rand::randFloat() * (M_PI * 2.0);
            
            float x = r * cos(rho) * sin(theta);
            float y = r * sin(rho) * sin(theta);
            float z = r * cos(theta);
            
            float mass = Rand::randFloat(0.01f,1.0f);
            positions.push_back(Vec4f(x,y,z,mass));
        }
        mPredatorController.addFormation(new ParticleFormation("random2", bufSize, positions, velocities, data));
    }
    
    mPredatorController.resetToFormation(0);
}

void Flock2::updatePredators(double dt)
{
    // TODO: refactor
    PingPongFbo& fbo = mPredatorController.getParticleFbo();
    
    gl::pushMatrices();
    gl::setMatricesWindow( fbo.getSize(), false ); // false to prevent vertical flipping
    gl::setViewport( fbo.getBounds() );
    
    fbo.bindUpdate();
    
    mPredatorController.getFormation().getPositionTex().bind(3);
    mPredatorController.getFormation().getVelocityTex().bind(4);
  
    if (mAudioInputHandler.hasTexture())
    {
        mAudioInputHandler.getFbo().bindTexture(6);
    }
    
    float simdt = mTimeController.getDelta();
    mPredatorSimShader.bind();
    mPredatorSimShader.uniform( "positions", 0 );
    mPredatorSimShader.uniform( "velocities", 1 );
    mPredatorSimShader.uniform( "information", 2);
    mPredatorSimShader.uniform( "oPositions", 3);
	mPredatorSimShader.uniform( "oVelocities", 4);
    mPredatorSimShader.uniform( "audioData", 6);
    mPredatorSimShader.uniform( "gain", mAudioInputHandler.getGain());
    mPredatorSimShader.uniform( "dt", (float)simdt );
    mPredatorSimShader.uniform( "reset", mReset );
    mPredatorSimShader.uniform( "startAnim", mPredatorController.isStartingAnim() );
    mPredatorSimShader.uniform( "formationStep", mPredatorController.getFormationStep() );
    
    gl::drawSolidRect(fbo.getBounds());
    
    mPredatorSimShader.unbind();
    
    mPredatorController.getFormation().getPositionTex().unbind();
    mPredatorController.getFormation().getVelocityTex().unbind();
    
    fbo.unbindUpdate();
    gl::popMatrices();
}
