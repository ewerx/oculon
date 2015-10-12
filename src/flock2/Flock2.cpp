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

#include "DustRenderer.h"
#include "GravitonRenderer.h"
#include "FlockRenderer.h"
#include "LinesRenderer.h"

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
    mBounds = Vec3f(200.0f, 150.0f, 200.0f);
    
    // Predators
    mPredatorSimShader = loadVertAndFragShaders("lines_simulation_vert.glsl", "flock_predator_sim_frag.glsl");
    setupPredators(8);
    
    // simulation
    mSimulationShader = loadVertAndFragShaders("lines_simulation_vert.glsl", "flock_prey_sim_frag.glsl");
    mFormationShader = loadVertAndFragShaders("lines_simulation_vert.glsl", "formation_sim_frag.glsl");
    
    const int bufSize = 64;
    setupParticles(bufSize);
    
    // behavior
    mBehaviorSelector.addValue("take-formation");
    mBehaviorSelector.addValue("flocking");
    
    // rendering
//    mParticleController.addRenderer( new FlockRenderer() );
    mParticleController.addRenderer( new GravitonRenderer() );
    mParticleController.addRenderer( new LinesRenderer() );
//    mParticleController.addRenderer( new DustRenderer() );
    
    mPredatorController.addRenderer( new GravitonRenderer() );
    mPredatorController.addRenderer( new LinesRenderer() );
    
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
    console() << "[lines] initializing " << numParticles << " prey..." << std::endl;
    
    mParticleController.setup(bufSize);
    
    vector<Vec4f> positions;
    vector<Vec4f> velocities;
    vector<Vec4f> data;
    
    const float r = mBounds.x;//100.0f;
    
    // random
    {
        for (int i = 0; i < numParticles; ++i)
        {
//            int nodeIndex = Rand::randInt(mNodePositions.size());//Rand::randInt(mNodeBufSize*mNodeBufSize);
            
            // position + mass
            float x = r * Rand::randFloat(-1.0f,1.0f);
            float y = r * Rand::randFloat(-1.0f,1.0f);
            float z = r * Rand::randFloat(-1.0f,1.0f);
            float leadership = Rand::randFloat(0.7f,1.0f); // GENERAL EMOTIONAL STATE.
            positions.push_back(Vec4f(x,y,z,leadership));
            
            // velocity + age
            Vec3f vel = Rand::randVec3f() * 1.0f;
            float crowd = 1.0f;//Rand::randFloat(.007f,0.9f);
            velocities.push_back(Vec4f(vel.x,vel.y,vel.z,crowd));
            
            // extra info
//            float decay = Rand::randFloat(.01f,10.00f);
//            data.push_back(Vec4f(x,y,z,(float)nodeIndex));
            data.push_back(Vec4f::zero());
        }
        mParticleController.addFormation(new ParticleFormation("random", bufSize, positions, velocities, data));
        positions.clear();
        data.clear();
    }
    
    // shell
    {
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
            // extra info
            float decay = Rand::randFloat(.01f,10.00f);
            data.push_back(Vec4f(x,y,z,decay));
        }
        mParticleController.addFormation(new ParticleFormation("shell", bufSize, positions, velocities, data));
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
    
    mInterface->addEnum(CreateEnumParam("behavior", &mBehaviorSelector.mIndex)
                        .maxValue(mBehaviorSelector.mNames.size())
                        .isVertical()
                        .sendFeedback(), mBehaviorSelector.mNames)->registerCallback(&mParticleController, &ParticleController::onFormationChanged);
    mInterface->addParam(CreateVec3fParam("bounds", &mBounds, Vec3f::one()*10.0f, Vec3f::one()*400.0f));
    
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("Predators");
    mPredatorController.setupInterface(mInterface, getName());
    
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("Prey");
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
    
//    if (mAudioInputHandler.hasTexture())
//    {
//        mAudioInputHandler.getFbo().bindTexture(7);
//    }
    
    // bind predator position tex
    mPredatorController.getParticleFbo().bindTexture(5, 0);
    
    // bind lantern position tex
//    mLanternController.getParticleFbo().bindTexture(6, 0);
    
    float simdt = mTimeController.getDelta();
    
    gl::GlslProg shader;
    if (mBehaviorSelector() == kBehaviorFormation) {
        shader = mFormationShader;
    } else {
        shader = mSimulationShader;
    }
    
    ////
    shader.bind();
    shader.uniform( "positions", 0 );
    shader.uniform( "velocities", 1 );
    shader.uniform( "information", 2);
    shader.uniform( "oPositions", 3);
	shader.uniform( "oVelocities", 4);
  	shader.uniform( "predatorPositionTex", 5);
//    shader.uniform( "lanternsTex", 6);
    shader.uniform( "particleBufSize", (float)mParticleController.getFboSize());
    shader.uniform( "predatorBufSize", (float)mPredatorController.getFboSize());
    shader.uniform( "dt", (float)simdt );
    shader.uniform( "reset", mReset );
    shader.uniform( "startAnim", mParticleController.isStartingAnim() );
    shader.uniform( "formationStep", mParticleController.getFormationStep() );
    shader.uniform( "bounds", mBounds );
    
    gl::drawSolidRect(fbo.getBounds());
    
    shader.unbind();
    /////
    
//    mLanternController.getParticleFbo().unbindTexture();
    mPredatorController.getParticleFbo().unbindTexture();
    
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
        mPredatorController.draw(mApp->getViewportSize(), getCamera(), mAudioInputHandler);
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
    
    const float r = mBounds.x * 0.5f;//50.0f;
    
    // random
    {
        for (int i = 0; i < numPredators; ++i)
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
    
    //    if (mAudioInputHandler.hasTexture())
    //    {
    //        mAudioInputHandler.getFbo().bindTexture(7);
    //    }
    
    // bind prey position tex
    mParticleController.getParticleFbo().bindTexture(5, 0);
    
    // bind lantern position tex
//    mLanternController.getParticleFbo().bindTexture(6, 0);
    
    float simdt = mTimeController.getDelta();
    mPredatorSimShader.bind();
    mPredatorSimShader.uniform( "positions", 0 );
    mPredatorSimShader.uniform( "velocities", 1 );
    mPredatorSimShader.uniform( "information", 2);
    mPredatorSimShader.uniform( "oPositions", 3);
    mPredatorSimShader.uniform( "oVelocities", 4);
    mPredatorSimShader.uniform( "preyPositionTex", 5);
//    mPredatorSimShader.uniform( "lanternsTex", 6);
    mPredatorSimShader.uniform( "particleBufSize", (float)mParticleController.getFboSize());
    mPredatorSimShader.uniform( "predatorBufSize", (float)mPredatorController.getFboSize());
    mPredatorSimShader.uniform( "dt", (float)simdt );
    mPredatorSimShader.uniform( "reset", mReset );
    mPredatorSimShader.uniform( "startAnim", mParticleController.isStartingAnim() );
    mPredatorSimShader.uniform( "formationStep", mParticleController.getFormationStep() );
    mPredatorSimShader.uniform( "bounds", mBounds );
    
    gl::drawSolidRect(fbo.getBounds());
    
    mPredatorSimShader.unbind();
    
//    mLanternController.getParticleFbo().unbindTexture();
    mParticleController.getParticleFbo().unbindTexture();
    
    mPredatorController.getFormation().getPositionTex().unbind();
    mPredatorController.getFormation().getVelocityTex().unbind();
    
    fbo.unbindUpdate();
    gl::popMatrices();
}

#pragma mark - Lanterns

void Flock2::setupLanterns(const int bufSize)
{
    int numLanterns = bufSize*bufSize;
    console() << "[lines] initializing " << numLanterns << " lanterns..." << std::endl;
    
    mLanternController.setup(bufSize);
    
    vector<Vec4f> positions;
    vector<Vec4f> velocities;
    vector<Vec4f> data;
    
//    const float r = 100.0f;
    
    // random
    {
        for (int i = 0; i < numLanterns; ++i)
        {
//            float rho = Rand::randFloat() * (M_PI * 2.0);
//            float theta = Rand::randFloat() * (M_PI * 2.0);
//            
//            float x = r * cos(rho) * sin(theta);
//            float y = r * sin(rho) * sin(theta);
//            float z = r * cos(theta);
//            
//            float mass = Rand::randFloat(0.01f,1.0f);
            positions.push_back(Vec4f(0.0f,0.0f,0.0f,1.0f));
            
            // velocity + age
            float vx = Rand::randFloat(-.005f,.005f);
            float vy = Rand::randFloat(-.005f,.005f);
            float vz = Rand::randFloat(-.005f,.005f);
            float age = Rand::randFloat(.007f,0.9f);
            velocities.push_back(Vec4f(vx,vy,vz,age));
            
            // extra info
//            float decay = Rand::randFloat(.01f,10.00f);
            //data.push_back(Vec4f(x,y,z,decay));
            data.push_back(Vec4f::zero());
        }
        mLanternController.addFormation(new ParticleFormation("one", bufSize, positions, velocities, data));
    }
    
    mPredatorController.resetToFormation(0);
}

