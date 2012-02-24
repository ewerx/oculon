/*
 *  Magnetosphere.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-11-26.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Magnetosphere.h"
#include "Resources.h"
#include "AudioInput.h" // compile errors if this is not before App.h
#include "OculonApp.h"//TODO: fix this dependency
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;


Magnetosphere::Magnetosphere()
: Scene()
{
}

Magnetosphere::~Magnetosphere()
{
}

void Magnetosphere::setup()
{
#ifdef USE_OPENGL_CONTEXT
	mOpencl.setupFromOpenGL();
#else	
	opencl.setup(CL_DEVICE_TYPE_CPU, 2);
#endif	
	
	for(int i=0; i<NUM_PARTICLES; i++) 
    {
		clParticle &p = mParticles[i];
		p.vel.set(0, 0);
		p.mass = Rand::randFloat(0.5f, 1.0f);		
		mParticlesPos[i].set(Rand::randFloat(mApp->getViewportWidth()), Rand::randFloat(mApp->getViewportHeight()));
	}
	
	glGenBuffersARB(1, mVbo);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[0]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(float2) * NUM_PARTICLES, mParticlesPos, GL_DYNAMIC_COPY_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    
    fs::path clPath = App::getResourcePath("Particle.cl");
	mOpencl.loadProgramFromFile(clPath.string());
	mKernelUpdate = mOpencl.loadKernel("updateParticle");
	
	
	mClMemParticles.initBuffer(sizeof(clParticle) * NUM_PARTICLES, CL_MEM_READ_WRITE, mParticles);
#ifdef USE_OPENGL_CONTEXT
	mClMemPosVBO.initFromGLObject(mVbo[0]);
#else
	mClMemPosVBO.initBuffer(sizeof(Vec2) * NUM_PARTICLES, CL_MEM_READ_WRITE, particlesPos);
#endif	
	
	mKernelUpdate->setArg(0, mClMemParticles.getCLMem());
	mKernelUpdate->setArg(1, mClMemPosVBO.getCLMem());
	mKernelUpdate->setArg(2, mMousePos);
	mKernelUpdate->setArg(3, mDimensions);
	
	glPointSize(1);
    
    reset();
}

void Magnetosphere::reset()
{
}

void Magnetosphere::resize()
{
}

//void Magnetosphere::setupMidiMapping()
//{
    // setup MIDI inputs for learning
    //mMidiMap.registerMidiEvent("orb_gravity", MidiEvent::TYPE_VALUE_CHANGE, this, &Orbiter::handleGravityChange);
    //mMidiMap.registerMidiEvent("orb_timescale", MidiEvent::TYPE_VALUE_CHANGE, this, &Orbiter::handleGravityChange);
    //mMidiMap.beginLearning();
    // ... or load a MIDI mapping
    //mMidiInput.setMidiKey("gravity", channel, note);
//}

void Magnetosphere::setupParams(params::InterfaceGl& params)
{
    params.addText( "magneto", "label=`Magnetosphere`" );
    //params.addParam("Gravity Constant", &mGravityConstant, "step=0.00000000001 keyIncr== keyDecr=-");
    //params.addParam("Follow Target", &mFollowTargetIndex, "keyIncr=] keyDecr=[");
    //params.addParam("Time Scale", &mTimeScale, "step=86400.0 KeyIncr=. keyDecr=,");
    //params.addParam("Max Radius Mult", &Orbiter::sMaxRadiusMultiplier, "step=0.1");
    //params.addParam("Frames to Avg", &Orbiter::sNumFramesToAvgFft, "step=1");
    //params.addParam("Trails - Smooth", &Orbiter::sUseSmoothLines, "key=s");
    //params.addParam("Trails - Ribbon", &Orbiter::sUseTriStripLine, "key=t");
    //params.addParam("Trails - LengthFact", &Orbiter::sMinTrailLength, "keyIncr=l keyDecr=;");
    //params.addParam("Trails - Width", &Orbiter::sTrailWidth, "keyIncr=w keyDecr=q step=0.1");
    //params.addParam("Planet Grayscale", &Orbiter::sPlanetGrayScale, "keyIncr=x keyDecr=z step=0.05");
    //params.addParam("Real Sun Radius", &Orbiter::sDrawRealSun, "key=r");
    //params.addSeparator();
    //params.addParam("Frustum Culling", &mEnableFrustumCulling, "keyIncr=f");
}

void Magnetosphere::update(double dt)
{
    Scene::update(dt);
    
	mDimensions.x = mApp->getViewportWidth();
	mDimensions.y = mApp->getViewportHeight();
	
	mKernelUpdate->setArg(2, mMousePos);
	mKernelUpdate->setArg(3, mDimensions);
	mKernelUpdate->run1D(NUM_PARTICLES);
    
    updateAudioResponse();
    //updateHud();
    
    // debug info
    char buf[256];
    snprintf(buf, 256, "particles: %d", NUM_PARTICLES);
    mApp->getInfoPanel().addLine(buf, Color(0.75f, 0.5f, 0.5f));
}

//
// handleKeyDown
//
bool Magnetosphere::handleKeyDown(const KeyEvent& keyEvent)
{
    bool handled = true;
    
    switch (keyEvent.getChar()) 
    {
        case ' ':
            reset();
            break;
        case 'g':
            //generateParticles();
            break;
        case 'q':
            //mParticleController.toggleParticleDrawMode();
            break;
        default:
            handled = false;
            break;
    }
    
    return handled;
}

void Magnetosphere::handleMouseDown( const MouseEvent& event )
{
	mIsMousePressed = true;
	mMousePos.x = event.getPos().x;
    mMousePos.y = event.getPos().y;
}

void Magnetosphere::handleMouseUp( const MouseEvent& event )
{
	mIsMousePressed = false;
}

void Magnetosphere::handleMouseDrag( const MouseEvent& event )
{
	mMousePos.x = event.getPos().x;
    mMousePos.y = event.getPos().y;
}

//
//
//
void Magnetosphere::updateAudioResponse()
{
    AudioInput& audioInput = mApp->getAudioInput();
    std::shared_ptr<float> fftDataRef = audioInput.getFftDataRef();
    
    //unsigned int bandCount = audioInput.getFftBandCount();
    //float* fftBuffer = fftDataRef.get();
    
    //int bodyIndex = 0;
    
    //TODO
}

void Magnetosphere::draw()
{
    glPushMatrix();
    gl::setMatricesWindow( mApp->getWindowSize() );
    //gl::enableDepthWrite( true );
	//gl::enableDepthRead( true );
	//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
    glColor3f(1.0f, 1.0f, 1.0f);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[0]);
#ifdef USE_OPENGL_CONTEXT
	mOpencl.finish();
#else	
	opencl.readBuffer(sizeof(Vec2) * NUM_PARTICLES, clMemPosVBO, particlesPos);
	glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(Vec2) * NUM_PARTICLES, particlesPos);
#endif	
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, 0);
	glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	
    glPopMatrix();
}

void Magnetosphere::generateParticles()
{
    /*
    Vec3f pos = Vec3f::zero();
    Vec3f vel = Vec3f::zero();
    float emitterRadius = 5.0f;
    int depth = pos.y - 380;
    float per = depth/340.0f;
    //Vec3f vel = mEmitter.mVel * per;
    //vel.y *= 0.02f;
    int numParticlesToSpawn = Rand::randInt(100,250);
    if( Rand::randFloat() < 0.02f )
    {
        //numParticlesToSpawn *= 5;
    }
    //mParticleController.addParticles( numParticlesToSpawn, pos, vel * per, emitterRadius );
     */
}

