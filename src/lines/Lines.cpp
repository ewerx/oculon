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
#include "Parsec.h"

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
    mFormation = FORMATION_RANDOM;
    mMotion = MOTION_NOISE;
    
    mAudioTime = false;
    mAltRenderer = false;
    
    // simulation
    mSimulationShader = loadVertAndFragShaders("lines_simulation_vert.glsl", "lines_simulation_frag.glsl");
    
    setupFBO();
    
    // rendering
    mRenderer.setup(kBufSize);
    mGravitonRenderer.setup(kBufSize);
    generateFormationTextures();
    
    mDynamicTexture.setup(kBufSize, kBufSize);
    
    mCameraController.setup(mApp, CameraController::CAM_MANUAL|CameraController::CAM_SPLINE, CameraController::CAM_MANUAL);
    mApp->setCamera(Vec3f(480.0f, 0.0f, 0.0f), Vec3f(-1.0f, 0.0f, 0.0f), Vec3f(0.0f,1.0f,0.0f));
    
    // audio
    mAudioInputHandler.setup(true);
}

void Lines::setupFBO()
{
    console() << "[Lines] initializing " << kNumParticles << " particles, hang on!" << std::endl;
    //initialize buffer
	Surface32f posSurface = Surface32f(kBufSize,kBufSize,true);
	Surface32f velSurface = Surface32f(kBufSize,kBufSize,true);
	Surface32f infoSurface = Surface32f(kBufSize,kBufSize,true);
    
	Surface32f::Iter iterator = posSurface.getIter();
	
    Perlin perlin(32, clock() * .1f);
    
	while(iterator.line())
	{
		while(iterator.pixel())
		{
            // position + mass
            float x = Rand::randFloat(-1.0f,1.0f);
            float y = Rand::randFloat(-1.0f,1.0f);
            float z = Rand::randFloat(-1.0f,1.0f);
            float mass = Rand::randFloat(0.01f,1.0f);
			posSurface.setPixel(iterator.getPos(), ColorA(x,y,z,mass));
            
            // velocity + age
            float vx = Rand::randFloat(-.005f,.005f);
            float vy = Rand::randFloat(-.005f,.005f);
            float vz = Rand::randFloat(-.005f,.005f);
            float age = Rand::randFloat(.007f,0.9f);
			velSurface.setPixel(iterator.getPos(), ColorA(vx,vy,vz,age));
            
			// extra info
            float decay = Rand::randFloat(.01f,10.00f);
			infoSurface.setPixel(iterator.getPos(),
                                 ColorA(x,y,z,decay));
		}
	}
    
    std::vector<Surface32f> surfaces;
    surfaces.push_back( posSurface );
    surfaces.push_back( velSurface );
    surfaces.push_back( infoSurface );
    mParticlesFbo = PingPongFbo( surfaces );
    
    gl::Texture::Format format;
    format.setInternalFormat( GL_RGBA32F_ARB );
	
	mFormationPosTex[FORMATION_RANDOM] = gl::Texture(posSurface, format);
	mFormationPosTex[FORMATION_RANDOM].setWrap( GL_REPEAT, GL_REPEAT );
	mFormationPosTex[FORMATION_RANDOM].setMinFilter( GL_NEAREST );
	mFormationPosTex[FORMATION_RANDOM].setMagFilter( GL_NEAREST );
	
	mInitialVelTex = gl::Texture(velSurface, format);
	mInitialVelTex.setWrap( GL_REPEAT, GL_REPEAT );
	mInitialVelTex.setMinFilter( GL_NEAREST );
	mInitialVelTex.setMagFilter( GL_NEAREST );
}

void Lines::generateFormationTextures()
{
    //TODO: make setupFBO use this formation function
    
    //initialize buffer
	Surface32f posSurface = Surface32f(kBufSize,kBufSize,true);
	Surface32f velSurface = Surface32f(kBufSize,kBufSize,true);
    
	Surface32f::Iter iterator = posSurface.getIter();
    
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    
    bool pair = false;
    
    
	while(iterator.line())
	{
		while(iterator.pixel())
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
			posSurface.setPixel(iterator.getPos(), ColorA(x,y,z,mass));
            
//            // velocity
//            float vx = Rand::randFloat(-.005f,.005f);
//            float vy = Rand::randFloat(-.005f,.005f);
//            float vz = Rand::randFloat(-.005f,.005f);
//            float age = Rand::randFloat(.007f,0.9f);
//            
//			// velocity + age
//			velSurface.setPixel(iterator.getPos(), ColorA(vx,vy,vz,age));
            
            pair = !pair;
		}
	}
    
    gl::Texture::Format format;
    format.setInternalFormat( GL_RGBA32F_ARB );
	
	mFormationPosTex[FORMATION_STRAIGHT] = gl::Texture(posSurface, format);
	mFormationPosTex[FORMATION_STRAIGHT].setWrap( GL_REPEAT, GL_REPEAT );
	mFormationPosTex[FORMATION_STRAIGHT].setMinFilter( GL_NEAREST );
	mFormationPosTex[FORMATION_STRAIGHT].setMagFilter( GL_NEAREST );
	
//	mInitialVelTex = gl::Texture(velSurface, format);
//	mInitialVelTex.setWrap( GL_REPEAT, GL_REPEAT );
//	mInitialVelTex.setMinFilter( GL_NEAREST );
//	mInitialVelTex.setMagFilter( GL_NEAREST );
    
    // HACK
    Scene *scene = mApp->getScene("parsec");
    
    if (scene)
    {
        Surface32f starsSurface = Surface32f(kBufSize,kBufSize,true);
        
        Parsec *parsec = static_cast<Parsec*>(scene);
        mFormationPosTex[FORMATION_PARSEC] = parsec->getStarPositionsTexture();
    }
    else
    {
        // prevent crash if parsec not running
        mFormationPosTex[FORMATION_PARSEC] = mFormationPosTex[FORMATION_STRAIGHT];
    }
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
                         .minValue(0.001f)
                         .maxValue(3.0f)
                         .oscReceiver(mName));
    
    mInterface->gui()->addColumn();
    vector<string> motionNames;
#define MOTION_ENTRY( nam, enm ) \
    motionNames.push_back(nam);
    MOTION_TUPLE
#undef  MOTION_ENTRY
    mInterface->addEnum(CreateEnumParam( "motion", (int*)(&mMotion) )
                        .maxValue(MOTION_COUNT)
                        .isVertical()
                        .oscReceiver(mName)
                        .sendFeedback(), motionNames);
    
    vector<string> formationNames;
#define FORMATION_ENTRY( nam, enm ) \
    formationNames.push_back(nam);
    FORMATION_TUPLE
#undef  FORMATION_ENTRY
    mInterface->addEnum(CreateEnumParam( "formation", (int*)(&mFormation) )
                        .maxValue(FORMATION_COUNT)
                        .isVertical()
                        .oscReceiver(mName)
                        .sendFeedback(), formationNames)->registerCallback(this, &Lines::takeFormation);;
    
    mInterface->addParam(CreateFloatParam( "formation_step", mFormationStep.ptr() ));
    mFormationAnimSelector.setupInterface(mInterface, mName);
    
    mDynamicTexture.setupInterface(mInterface, mName);
    
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("display");
    //mInterface->addParam(CreateBoolParam("alt render", &mAltRenderer));
    mRenderer.setupInterface(mInterface, mName);
    //mGravitonRenderer.setupInterface(mInterface, mName);
    
    mCameraController.setupInterface(mInterface, mName);
    mAudioInputHandler.setupInterface(mInterface, mName);
}

#pragma mark - Callbacks

bool Lines::takeFormation()
{
    mFormationStep = 0.0f;
    timeline().apply( &mFormationStep, 1.0f, mFormationAnimSelector.mDuration,mFormationAnimSelector.getEaseFunction() );
    mFormationStep = 0.0f;
    return true;
}

#pragma mark - Update

void Lines::update(double dt)
{
    mAudioInputHandler.update(dt, mApp->getAudioInput(), mGain);
    
    mCameraController.update(dt);
    
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
    mFormationPosTex[mFormation].bind(4);
    mDynamicTexture.bindTexture(5);
    
    float simdt = (float)(dt*mTimeStep);
    if (mAudioTime) simdt *= (1.0 - mAudioInputHandler.getAverageVolumeLowFreq());
    mSimulationShader.bind();
    mSimulationShader.uniform( "positions", 0 );
    mSimulationShader.uniform( "velocities", 1 );
    mSimulationShader.uniform( "information", 2);
	mSimulationShader.uniform( "oVelocities", 3);
	mSimulationShader.uniform( "oPositions", 4);
  	mSimulationShader.uniform( "noiseTex", 5);
    mSimulationShader.uniform( "dt", (float)dt );
    mSimulationShader.uniform( "reset", mReset );
    mSimulationShader.uniform( "formationStep", mFormationStep );
    mSimulationShader.uniform( "motion", mMotion );
    mSimulationShader.uniform( "containmentSize", 3.0f );
    
    gl::drawSolidRect(mParticlesFbo.getBounds());
    mSimulationShader.unbind();
    
    mDynamicTexture.unbindTexture();
    mFormationPosTex[mFormation].unbind();
    mInitialVelTex.unbind();
    
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

ParticleRenderer& Lines::getRenderer()
{
    if (mAltRenderer)
    {
        return mGravitonRenderer;
    }
    else
    {
        return mRenderer;
    }
}

void Lines::draw()
{
    ParticleRenderer& renderer = getRenderer();
    
    gl::pushMatrices();
    if (mApp->outputToOculus())
    {
        // render left eye
        Area leftViewport = Area( Vec2f( 0.0f, 0.0f ), Vec2f( getFbo().getWidth() / 2.0f, getFbo().getHeight() ) );
        gl::setViewport(leftViewport);
        mApp->getOculusCam().enableStereoLeft();
        renderer.draw(mParticlesFbo, leftViewport.getSize(), mApp->getOculusCam().getCamera(), mAudioInputHandler, mGain);
        
        Area rightViewport = Area( Area( Vec2f( getFbo().getWidth() / 2.0f, 0.0f ), Vec2f( getFbo().getWidth(), getFbo().getHeight() ) ) );
        gl::setViewport(rightViewport);
        mApp->getOculusCam().enableStereoRight();
        renderer.draw(mParticlesFbo, rightViewport.getSize(), mApp->getOculusCam().getCamera(), mAudioInputHandler, mGain);
    }
    else
    {
        renderer.draw(mParticlesFbo, mApp->getViewportSize(), getCamera(), mAudioInputHandler, mGain);
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
    
    Rectf preview( windowSize.x - (size+paddingX), windowSize.y - (size+paddingY), windowSize.x-paddingX, windowSize.y - paddingY );
    gl::draw( mParticlesFbo.getTexture(0), preview );
    
    Rectf preview2 = preview - Vec2f(size+paddingX, 0.0f);
    gl::draw( mParticlesFbo.getTexture(1), preview2 );
    
    Rectf preview3 = preview2 - Vec2f(size+paddingX, 0.0f);
    gl::draw(mDynamicTexture.getTexture(), preview3);
    
    glPopAttrib();
    gl::popMatrices();
}
