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
    
    // params
    mTimeStep = 0.1f;
    mDecayRate = 0.5f;
    
    mAudioReactive = false;
    mUseDynamicTex = true;
    
    mAudioTime = false;
    
    // shaders
    mSimulationShader = loadVertAndFragShaders("dust_simulation_vert.glsl", "dust_simulation_frag.glsl");
    
    setupFBO();
    mRenderer.setup(kBufSize);
    mLinesRenderer.setup(kBufSize);
    
    mAudioInputHandler.setup(false);
    
    mDynamicTexture.setup(kBufSize, kBufSize);
    
    mCamera.setOrtho( 0, mApp->getViewportWidth(), mApp->getViewportHeight(), 0, -1, 1 );
}

void Dust::setupFBO()
{
    console() << "[dust] initializing " << kNumParticles << " particles, hang on!" << std::endl;
    //initialize buffer
	Surface32f posSurface = Surface32f(kBufSize,kBufSize,true);
	Surface32f velSurface = Surface32f(kBufSize,kBufSize,true);
	Surface32f infoSurface = Surface32f(kBufSize,kBufSize,true);
    Surface32f noiseSurface = Surface32f(kBufSize,kBufSize,true);
    
	Surface32f::Iter iterator = posSurface.getIter();
	
    Perlin perlin(32, clock() * .1f);
    
	while(iterator.line())
	{
		while(iterator.pixel())
		{
            float x = Rand::randFloat(mApp->getViewportWidth()) / (float)mApp->getViewportWidth();
            float y = Rand::randFloat(mApp->getViewportHeight()) / (float)mApp->getViewportHeight();
            float z = 0.0f;
            float mass = Rand::randFloat(0.01f,1.0f);
            
            // position + mass
			posSurface.setPixel(iterator.getPos(), ColorA(x,y,z,mass));
            
            // velocity
            float vx = Rand::randFloat(-.005f,.005f);
            float vy = Rand::randFloat(-.005f,.005f);
            float vz = 1.0f;
            float age = Rand::randFloat(.007f,0.9f);
            
			// velocity + age
			velSurface.setPixel(iterator.getPos(), ColorA(vx,vy,vz,age));
            
			// decay + max age
            float decay = Rand::randFloat(.01f,10.00f);
            float maxAge = Rand::randFloat(1.0f,10.0f);
			infoSurface.setPixel(iterator.getPos(),
                                 ColorA(decay, maxAge, 0.0f, 0.0f));
            
            // noise
            float nX = iterator.x() * 0.005f;
            float nY = iterator.y() * 0.005f;
            float nZ = app::getElapsedSeconds() * 0.1f;
            Vec3f v( nX, nY, nZ );
            float noise = perlin.fBm( v );
            
            float angle = noise * 15.0f;
            
            noiseSurface.setPixel(iterator.getPos(),
                                  ColorA( cos( angle ) * Rand::randFloat(.1f,.4f), sin( angle ) * Rand::randFloat(.1f,.4f), 0.25f, 1.0f ));
		}
	}
    
    std::vector<Surface32f> surfaces;
    surfaces.push_back( posSurface );
    surfaces.push_back( velSurface );
    surfaces.push_back( infoSurface );
    mParticlesFbo = PingPongFbo( surfaces );
    
    gl::Texture::Format format;
    format.setInternalFormat( GL_RGBA32F_ARB );
    
    mNoiseTex = gl::Texture(noiseSurface, format);
	mNoiseTex.setWrap( GL_REPEAT, GL_REPEAT );
	mNoiseTex.setMinFilter( GL_NEAREST );
	mNoiseTex.setMagFilter( GL_NEAREST );
	
	mInitialPosTex = gl::Texture(posSurface, format);
	mInitialPosTex.setWrap( GL_REPEAT, GL_REPEAT );
	mInitialPosTex.setMinFilter( GL_NEAREST );
	mInitialPosTex.setMagFilter( GL_NEAREST );
	
	mInitialVelTex = gl::Texture(velSurface, format);
	mInitialVelTex.setWrap( GL_REPEAT, GL_REPEAT );
	mInitialVelTex.setMinFilter( GL_NEAREST );
	mInitialVelTex.setMagFilter( GL_NEAREST );
    
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
    
    mInterface->addParam(CreateBoolParam("dynamic_noise", &mUseDynamicTex)
                         .oscReceiver(mName));
    mDynamicTexture.setupInterface(mInterface, mName);
    
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("display");
    mInterface->addParam(CreateBoolParam("alt render", &mAltRenderer));
    mRenderer.setupInterface(mInterface, mName);
    mLinesRenderer.setupInterface(mInterface, mName);
    
//    mInterface->addParam(CreateBoolParam("audioreactive", &mAudioReactive));
    //mInterface->addParam(CreateBoolParam("audiospeed", &mAudioTime));
    
    mAudioInputHandler.setupInterface(mInterface, mName);
}

#pragma mark - Update

void Dust::update(double dt)
{
    mAudioInputHandler.update(dt, mApp->getAudioInput(), mGain);
    
    gl::disableAlphaBlending();
    
    gl::disableAlphaBlending();
	gl::disableDepthRead();
	gl::disableDepthWrite();

    mDynamicTexture.update(dt);
    
    gl::pushMatrices();
    gl::setMatricesWindow( mParticlesFbo.getSize(), false ); // false to prevent vertical flipping
    gl::setViewport( mParticlesFbo.getBounds() );
    
    mParticlesFbo.bindUpdate();
    
    mInitialVelTex.bind(3);
    mInitialPosTex.bind(4);
    //mNoiseTex.bind(5);
    
    if (mUseDynamicTex)
    {
        mDynamicTexture.bindTexture(5);
    }
    else
    {
        mNoiseTex.bind(5);
    }
    
    float simdt = (float)(dt*mTimeStep);
    float decayRate = mDecayRate;
    if (mAudioTime) simdt *= (1.0 - mAudioInputHandler.getAverageVolumeLowFreq());
    if (mAudioTime) decayRate = mDecayRate + mAudioInputHandler.getAverageVolumeMidFreq();
    mSimulationShader.bind();
    mSimulationShader.uniform( "positions", 0 );
    mSimulationShader.uniform( "velocities", 1 );
    mSimulationShader.uniform( "information", 2);
	mSimulationShader.uniform( "oVelocities", 3);
	mSimulationShader.uniform( "oPositions", 4);
  	mSimulationShader.uniform( "noiseTex", 5);
    mSimulationShader.uniform( "dt", simdt );
    mSimulationShader.uniform( "decayRate", decayRate );
    mSimulationShader.uniform( "reset", mReset );
    //mSimulationShader.uniform( "takeFormation", mTakeFormation );
    
    gl::drawSolidRect(mParticlesFbo.getBounds());
    mSimulationShader.unbind();
    
    if (mUseDynamicTex)
    {
        mDynamicTexture.unbindTexture();
    }
    else
    {
        mNoiseTex.unbind();
    }
    mInitialPosTex.unbind();
    mInitialVelTex.unbind();
    
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

ParticleRenderer& Dust::getRenderer()
{
    if (mAltRenderer)
    {
        return mLinesRenderer;
    }
    else
    {
        return mRenderer;
    }
}

void Dust::draw()
{
    ParticleRenderer& renderer = getRenderer();
    
    gl::pushMatrices();
    renderer.draw(mParticlesFbo, mApp->getViewportSize(), getCamera(), mAudioInputHandler, mGain);
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
    
    Rectf preview( windowSize.x - (size+paddingX), windowSize.y - (size+paddingY), windowSize.x-paddingX, windowSize.y - paddingY );
    gl::draw( mParticlesFbo.getTexture(0), preview );
    
    Rectf preview2 = preview - Vec2f(size+paddingX, 0.0f);
    gl::draw( mParticlesFbo.getTexture(1), preview2 );
    
    Rectf preview3 = preview2 - Vec2f(size+paddingX, 0.0f);
    if (mUseDynamicTex)
    {
        gl::draw(mDynamicTexture.getTexture(), preview3);
    }
    else
    {
        gl::draw(mNoiseTex, preview3);
    }
    
    glPopAttrib();
    gl::popMatrices();
}
