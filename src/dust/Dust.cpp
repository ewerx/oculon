//
//  Dust.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 11/21/2013.
//
//

#include "Dust.h"
#include "OculonApp.h"
#include "Interface.h"
#include "cinder/Perlin.h"
#include "cinder/Rand.h"
#include "ParticleFormation.h"

#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#pragma mark - Construction

Dust::Dust()
: Scene("dust")
{
}

Dust::~Dust()
{
}

#pragma mark - Setup

void Dust::setup()
{
    Scene::setup();
    
    mReset = false;
    
    // should match simulaion shader
    mMotionTypes.push_back( "static" );
    mMotionTypes.push_back( "noise" );
    mMotion = 1;
    
    // params
    mTimeStep = 0.1f;
    mDecayRate = 0.5f;
    
    mAudioReactive = false;
    
    mAudioTime = false;
    
    // simulation
    mSimulationShader = loadVertAndFragShaders("dust_simulation_vert.glsl", "dust_simulation_frag.glsl");
    
    const int bufSize = 512;
    setupParticles(bufSize);
    
    // rendering
    mParticleController.addRenderer( new DustRenderer() );
    
    mDynamicTexture.setup(bufSize, bufSize);
    
    mCamera.setOrtho( 0, mApp->getViewportWidth(), mApp->getViewportHeight(), 0, -1, 1 );
    
    // audio
    mAudioInputHandler.setup(true);
}

void Dust::setupParticles(const int bufSize)
{
    mParticleController.setup(bufSize);
    int numParticles = bufSize*bufSize;
    
    console() << "[dust] initializing " << numParticles << " particles, hang on!" << std::endl;
    
    const float maxPosX = mApp->getViewportWidth() / (float)mApp->getViewportWidth();
    
    Vec4f posMin(0.0f,      0.0f,       0.0f,   0.01f);
    Vec4f posMax(maxPosX,    1.0f,      0.0f,   1.0f);
    Vec4f velMin(-0.005f,   -0.005f,    0.0f,   0.007f);// age
    Vec4f velMax(0.005f,    0.005f,     1.0f,   0.9f);
    Vec4f dataMin(0.01f,    1.0f,       0.0f,   0.0f);
    Vec4f dataMax(10.0f,    10.0f,      1.0f,   1.0f);
    mParticleController.addFormation(new RandomFormation(bufSize, posMin, posMax,
                                                         velMin, velMax,
                                                         dataMin, dataMax));
    
    // chrysanthemum
    int i;
    double u,r,p4,p8;
    Vec3f p;
    
    vector<Vec4f> positions;
    vector<Vec4f> velocities;
    vector<Vec4f> data;
    
    for (i=0;i<=numParticles;i++)
    {
        u = i * 21.0f * M_PI / numParticles;
        p4 = sin(17.f * u / 3.f);
        p8 = sin(2.f * cos(3.f * u) - 28.f * u);
        r = 0.5f*(1.f + sin(11.f*u/5.f)) - 4.f*p4*p4*p4*p4 * p8*p8*p8*p8*p8*p8*p8*p8;
        float x = r * cos(u)+0.5f;
        float y = r * sin(u)+0.25f;
        float z = (r/20.f+.2f)*sin(r*2.0f*M_PI/7.0f);
        
        positions.push_back(Vec4f(x,y,z,1.0f));
        velocities.push_back(Vec4f::zero());
        data.push_back(Vec4f::zero());
    }
    mParticleController.addFormation(new ParticleFormation("chrysanthemum", bufSize, positions, velocities, data));
    
    mParticleController.resetToFormation(0);
}

void Dust::reset()
{
    mReset = true;
}

#pragma mark - Interface

void Dust::setupInterface()
{
    mInterface->addParam(CreateFloatParam( "timestep", &mTimeStep )
                         .minValue(0.001f)
                         .maxValue(3.0f)
                         .oscReceiver(mName));
    
    mInterface->addParam(CreateFloatParam( "decay_rate", &mDecayRate )
                         .minValue(0.0f)
                         .maxValue(1.0f)
                         .oscReceiver(mName));
    
    mInterface->addEnum(CreateEnumParam( "motion", &mMotion )
                        .maxValue(mMotionTypes.size())
                        .oscReceiver(getName())
                        .isVertical(), mMotionTypes);
    mDynamicTexture.setupInterface(mInterface, mName);
    
    mInterface->gui()->addColumn();
    mParticleController.setupInterface(mInterface, mName);

//    mInterface->addParam(CreateBoolParam("audioreactive", &mAudioReactive));
    //mInterface->addParam(CreateBoolParam("audiospeed", &mAudioTime));
    
    mAudioInputHandler.setupInterface(mInterface, mName);
}

#pragma mark - Update

void Dust::update(double dt)
{
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
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
    float decayRate = mDecayRate;
    if (mAudioTime) simdt *= (1.0 - mAudioInputHandler.getAverageVolumeLowFreq());
    if (mAudioTime) decayRate = mDecayRate + mAudioInputHandler.getAverageVolumeMidFreq();
    mSimulationShader.bind();
    mSimulationShader.uniform( "positions", 0 );
    mSimulationShader.uniform( "velocities", 1 );
    mSimulationShader.uniform( "information", 2);
    mSimulationShader.uniform( "oPositions", 3);
	mSimulationShader.uniform( "oVelocities", 4);
  	mSimulationShader.uniform( "noiseTex", 5);
    mSimulationShader.uniform( "dt", simdt );
    mSimulationShader.uniform( "decayRate", decayRate );
    mSimulationShader.uniform( "reset", mReset );
    mSimulationShader.uniform( "formationStep", mParticleController.getFormationStep() );
    mSimulationShader.uniform( "motion", mMotion );
    
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

const Camera& Dust::getCamera()
{
    return mCamera;
}

void Dust::draw()
{
    gl::pushMatrices();
    mParticleController.draw(mApp->getViewportSize(), getCamera(), mAudioInputHandler);
    gl::popMatrices();
}

void Dust::drawDebug()
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
