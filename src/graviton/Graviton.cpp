/*
 *  Graviton.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-11-26.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Graviton.h"
#include "Resources.h"
#include "AudioInput.h" // compile errors if this is not before App.h
#include "OculonApp.h"//TODO: fix this dependency
#include "cinder/Rand.h"
#include "cinder/Easing.h"
#include "Utils.h"
#include "Orbiter.h"

using namespace ci;
using namespace ci::app;

#define PARTICLE_FLAGS_NONE         0x00
#define PARTICLE_FLAGS_INVSQR       (1 << 0)
#define PARTICLE_FLAGS_SHOW_DARK    (1 << 1)
#define PARTICLE_FLAGS_SHOW_SPEED   (1 << 2)
#define PARTICLE_FLAGS_SHOW_MASS    (1 << 3)


Graviton::Graviton()
: Scene()
{
}

Graviton::~Graviton()
{
}

void Graviton::setup()
{
    mTimeStep = 0.00075f;
    mGravity = 50.0f;
    
    mUseInvSquareCalc = true;
    mFlags = PARTICLE_FLAGS_NONE;
    
    mInitialFormation = FORMATION_SPHERE;
    mFormationRadius = 50.0f;
    
    mAdditiveBlending = true;
    mEnableLineSmoothing = false;
    mEnablePointSmoothing = false;
    mUseImageForPoints = true;
    mPointSize = 2.0f;
    mParticleAlpha = 0.45f;
    
    mDamping = 1.0f;
    mGravity = 100.0f;
    mEps = 0.01f;//mFormationRadius * 0.5f;
    
    mNumNodes = 0;
    mGravityNodeFormation = NODE_FORMATION_NONE;
    
    // camera
    mCamRadius = mFormationRadius * 3.0f;
    mCamAngle = 0.0f;
    mCamMaxDistance = mFormationRadius * 3.0f;
    mCamLateralPosition = -mCamMaxDistance;
    mCamTarget = Vec3f::zero();
    mCamTurnRate = 0.25f;
    mCamTranslateRate = 10.f;
    mCamType = CAM_SPIRAL;
    
    
    if( gl::isExtensionAvailable("glPointParameterfARB") && gl::isExtensionAvailable("glPointParameterfvARB") )
    {
        mScalePointsByDistance = true;
    }
    else
    {
        // it works anyway
        mScalePointsByDistance = true;
    }
    
	initOpenCl();
    
    mParticleTexture = gl::Texture( loadImage( app::loadResource( RES_GLITTER ) ) );
    mParticleTexture.setWrap( GL_REPEAT, GL_REPEAT );
    
    // blur shader
    try 
    {
		mBlurShader = gl::GlslProg( loadResource( RES_BLUR2_VERT ), loadResource( RES_BLUR2_FRAG ) );
	}
	catch( gl::GlslProgCompileExc &exc ) 
    {
		std::cout << "Shader compile error: " << std::endl;
		std::cout << exc.what();
	}
	catch( ... ) 
    {
		std::cout << "Unable to load shader" << std::endl;
	}
    
    reset();
}

//void Graviton::setupMidiMapping()
//{
// setup MIDI inputs for learning
//mMidiMap.registerMidiEvent("orb_gravity", MidiEvent::TYPE_VALUE_CHANGE, this, &Orbiter::handleGravityChange);
//mMidiMap.registerMidiEvent("orb_timescale", MidiEvent::TYPE_VALUE_CHANGE, this, &Orbiter::handleGravityChange);
//mMidiMap.beginLearning();
// ... or load a MIDI mapping
//mMidiInput.setMidiKey("gravity", channel, note);
//}

void Graviton::setupParams(params::InterfaceGl& params)
{
    params.addText( "graviton", "label=`Graviton`" );
    params.addParam("Inv Square", &mUseInvSquareCalc );
    params.addParam("Time Step_", &mTimeStep, "step=0.0001 min=0.0 max=1.0" );
    params.addParam("Initial Formation", (int*)(&mInitialFormation), "min=0 max=3");//"enum='0 {Sphere}, 1 {Shell}, 2 {Disc}, 3 {Galaxy}' " );
    params.addParam("Node Formation", (int*)(&mGravityNodeFormation), "min=0 max=2");
    params.addParam("Formation Radius", &mFormationRadius, "min=1.0" );
    params.addParam("Damping", &mDamping, "min=0.0 step=0.0001");
    params.addParam("Gravity", &mGravity, "min=0.0 max=1000 step=0.1");
    params.addParam("EPS", &mEps, "min=0.01 max=1000 step=1)");
    
    params.addParam("Cam Type", (int*)(&mCamType), "min=0 max=2");
    params.addParam("Cam Radius", &mCamRadius, "min=1.0");
    params.addParam("Cam Distance", &mCamMaxDistance, "min=0.0");
    params.addParam("Cam Turn Rate", &mCamTurnRate, "step=0.01");
    params.addParam("Cam Slide Rate", &mCamTranslateRate, "");
    
    params.addParam("Point Size", &mPointSize, "");
    params.addParam("Point Smoothing", &mEnablePointSmoothing, "");
    params.addParam("Point Sprites", &mUseImageForPoints, "");
    params.addParam("Point Scaling", &mScalePointsByDistance, "");
    params.addParam("Additive Blending", &mAdditiveBlending, "");
    params.addParam("Motion Blur", &mUseMotionBlur);
    params.addParam("Alpha", &mParticleAlpha, "min=0.0 max=1.0 step=0.001");

}

void Graviton::initParticles()
{
    mStep = kStep;
    mNumParticles = kNumParticles;
    
    const double r = mFormationRadius;
    
    for( size_t i = 0; i < kNumParticles; ++i )
    {
        if( (NODE_FORMATION_NONE != mGravityNodeFormation) && i < mNumNodes)
        {
            mPosAndMass[i].x = mGravityNodes[i].mPos.x;
            mPosAndMass[i].y = mGravityNodes[i].mPos.y;
            mPosAndMass[i].z = mGravityNodes[i].mPos.z;
            mPosAndMass[i].w = 1.0f;
            
            mVel[i].x = mGravityNodes[i].mVel.x;
            mVel[i].x = mGravityNodes[i].mVel.y;
            mVel[i].x = mGravityNodes[i].mVel.z;
            mVel[i].w = mGravityNodes[i].mMass;
            
            mColor[i].x = 1.0f;
            mColor[i].y = 1.0f;
            mColor[i].z = 1.0f;
            mColor[i].w = 1.0f;
        }
        else
        {
            double x = 0.0f;
            double y = 0.0f;
            double z = 0.0f;
            
            double vx = 0.0f;
            double vy = 0.0f;
            double vz = 0.0f;
            
            double rho = 0.0f;
            double theta = 0.0f;
            
            const float maxMass = 50.0f;
            float mass = Rand::randFloat(1.0f,maxMass);
            
            switch( mInitialFormation )
            {
                case FORMATION_SPHERE:
                {
                    rho = Utils::randDouble() * (M_PI * 2.0);
                    theta = Utils::randDouble() * (M_PI * 2.0);
                    
                    const float d = Rand::randFloat(10.0f, r);
                    x = d * cos(rho) * sin(theta);
                    y = d * sin(rho) * sin(theta);
                    z = d * cos(theta);
                }
                    break;
                    
                case FORMATION_SPHERE_SHELL:
                {
                    rho = Utils::randDouble() * (M_PI * 2.0);
                    theta = Utils::randDouble() * (M_PI * 2.0);
                    
                    x = r * cos(rho) * sin(theta);
                    y = r * sin(rho) * sin(theta);
                    z = r * cos(theta);
                }
                    break;
                    
                case FORMATION_DISC:
                {
                    rho = r * Utils::randDouble();//pow(Utils::randDouble(), 0.75);
                    theta = Utils::randDouble() * (M_PI * 2.0);
                    theta = (0.5 * cos(2.0 * theta) + theta - 1e-2 * rho);
                    
                    const float thickness = 1.0f;
                    
                    x = rho * cos(theta);
                    y = rho * sin(theta);
                    z = thickness * 2.0 * Utils::randDouble() - 1.0;
                    
                    const double a = 1.0e0 * (rho <= 1e-1 ? 0.0 : rho);
                    vx = -a * sin(theta);
                    vy = a * cos(theta);
                    vz = 0.0f;
                }
                    break;
                    
                case FORMATION_GALAXY:
                {
                    rho = r * pow(Utils::randDouble(), 0.75);
                    theta = Utils::randDouble() * (M_PI * 2.0);
                    theta = (0.5 * cos(2.0 * theta) + theta - 1e-2 * rho);
                    
                    x = rho * cos(theta);
                    y = rho * sin(theta);
                    
                    const float dist = sqrt(x*x + y*y);
                    const float maxThickness = r / 8.0f;
                    const float coreDistanceRatio = EaseInOutQuad()(1.0f - dist / r);
                    const float thickness =  maxThickness * coreDistanceRatio;
                    
                    z = thickness * (2.0 * Utils::randDouble() - 1.0);
                    
                    const double a = 1.0e0 * (rho <= 1e-1 ? 0.0 : rho);
                    vx = -a * sin(theta);
                    vy = a * cos(theta);
                    vz = 0.0f;
                    
                    mass = maxMass * coreDistanceRatio;
                }
                    break;
                    
                default:
                    break;
            }
            
            // pos
            mPosAndMass[i].x = x;
            mPosAndMass[i].y = y;
            mPosAndMass[i].z = z;
            mPosAndMass[i].w = 1.0f; //scale??
            
            // vel
            
            mVel[i].x = vx;
            mVel[i].y = vy;
            mVel[i].z = vz;
            mVel[i].w = mass;
            
            // color
            mColor[i].x = 1.0f;
            mColor[i].y = 1.0f;
            mColor[i].z = 1.0f;
            mColor[i].w = 1.0f;
        }
    }
}
                              
void Graviton::resetGravityNodes(const eNodeFormation formation)
{
    mGravityNodes.clear();
    
    switch( formation )
    {
        case NODE_FORMATION_SYMMETRY:
        {
            mNumNodes = 3;
            const float r = mFormationRadius;
            
            for(int i = 0; i < mNumNodes; ++i )
            {
                Vec3f pos;
                
                switch(i)
                {
                    case 0:
                        pos.x = 0.0f;
                        pos.y = 0.0f;
                        pos.z = 0.0f;
                        break;
                        
                    case 1:
                        
                        pos.x = r*0.5f;
                        pos.y = -r*0.5f;
                        pos.z = r / 4.0f;
                        break;
                    case 2:
                        pos.x = -r*0.5f;
                        pos.y = r*0.5f;
                        pos.z = -r / 4.0f;
                        break;
                }
                
                const float mass = 10000.f;
                mGravityNodes.push_back( tGravityNode( pos, Vec3f::zero(), mass ) );
            }
        } break;
            
            
        case NODE_FORMATION_BLACKHOLE_STAR:
        {
            mNumNodes = 1;
            
            const float mass = 10000.f;
            const float d = mFormationRadius * 3.0f;
            
            //Vec3f pos( d, d, d );
            Vec3f pos(0,0,0);
            mGravityNodes.push_back( tGravityNode( pos, Vec3f::zero(), mass ) );
        } break;
            
        default:
            break;
    }

}
                              

void Graviton::initOpenCl()
{
    mOpenCl.setupFromOpenGL();
    
    const size_t size = sizeof(float4) * kNumParticles;
    
    // init VBO
    //
    glGenBuffersARB(2, mVbo); // 2 VBOs, color and position
    
    // use VBO instead of CL buffer for positions to render directly
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[0]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, mPosAndMass, GL_STREAM_COPY_ARB);
	glVertexPointer(4, GL_FLOAT, 0, 0);
	
    // colors store in another vbo
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[1]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, mColor, GL_STREAM_COPY_ARB);
	glColorPointer(4, GL_FLOAT, 0, 0);
    
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    
    
    // init CL kernel
    //
    fs::path clPath = App::getResourcePath("Nbody.cl");
	mOpenCl.loadProgramFromFile(clPath.string());
	mKernel = mOpenCl.loadKernel("gravity");
	
    // init CL buffers
    //
    //mClBufPos0.initBuffer( size, CL_MEM_READ_WRITE, mPosAndMass );
    mClBufPos0.initFromGLObject(mVbo[0]);
    mClBufPos1.initBuffer( size, CL_MEM_READ_ONLY );
    mClBufVel0.initBuffer( size, CL_MEM_READ_WRITE );
    mClBufVel1.initBuffer( size, CL_MEM_READ_ONLY );
    mClBufColor.initFromGLObject(mVbo[1]);
    mClBufFft.initBuffer( sizeof(cl_float)*kFftBands, CL_MEM_READ_WRITE );

}

void Graviton::reset()
{
    resetGravityNodes(mGravityNodeFormation);
    initParticles();
    
    const size_t size = sizeof(float4) * kNumParticles;
	
    // recreate teh buffers (afaik there's no leak here)
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[0]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, mPosAndMass, GL_STREAM_COPY_ARB);
	glVertexPointer(4, GL_FLOAT, 0, 0);
	
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[1]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, mColor, GL_STREAM_COPY_ARB);
	glColorPointer(4, GL_FLOAT, 0, 0);
    
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    
    mClBufVel0.write( mVel, 0, sizeof(float4) * kNumParticles );
    
#if defined( FREEOCL_VERSION )
    mKernel->setArg(ARG_POS_IN, mClBufPos0.getCLMem());
    mKernel->setArg(ARG_POS_OUT, mClBufPos1.getCLMem());
    mKernel->setArg(ARG_VEL_IN, mClBufVel0.getCLMem());
    mKernel->setArg(ARG_VEL_OUT, mClBufVel1.getCLMem());
    mKernel->setArg(ARG_COLOR, mClBufColor.getCLMem());
    mKernel->setArg(ARG_FFT, mClBufFft.getCLMem());
    mKernel->setArg(ARG_COUNT, mNumParticles);
    mKernel->setArg(ARG_STEP, mStep);
#else
    //TODO
    mKernel->setArg(ARG_POS_IN, mSwap ? mClBufPos1.getCLMem() : mClBufPos0.getCLMem());
    mKernel->setArg(ARG_POS_OUT, mSwap ? mClBufPos0.getCLMem() : mClBufPos1.getCLMem());
    mKernel->setArg(ARG_VEL, mClBufVel0.getCLMem());
#endif
    
    mSwap = false;
    
    // camera
    mCamAngle = 0.0f;
    mCamLateralPosition = -mCamMaxDistance;
    mCamTarget = Vec3f::zero();
    mCam.setFov(60.0f);
    mCam.setAspectRatio(mApp->getWindowAspectRatio());
    
    mMotionBlurRenderer.setup( mApp->getWindowSize(), boost::bind( &Graviton::drawParticles, this ) );
}

void Graviton::resize()
{
    mMotionBlurRenderer.resize(mApp->getWindowSize());
}

void Graviton::update(double dt)
{
	//mDimensions.x = mApp->getViewportWidth();
	//mDimensions.y = mApp->getViewportHeight();
    
    updateAudioResponse();

#if defined( FREEOCL_VERSION )
    //mAnimTime += mTimeSpeed;
    //mKernelUpdate->setArg(ARG_TIME, mAnimTime);
    
    // mStep = # of particles that act as gravitational attractors
    mStep = (NODE_FORMATION_NONE != mGravityNodeFormation) ? mNumNodes : (mNumParticles / kStep) / 4;

    mKernel->setArg(ARG_DT, mTimeStep);
    mKernel->setArg(ARG_COUNT, mNumParticles);
    mKernel->setArg(ARG_STEP, mStep);
    mKernel->setArg(ARG_DAMPING, mDamping);
    mKernel->setArg(ARG_GRAVITY, mGravity);
    mKernel->setArg(ARG_ALPHA, mParticleAlpha);
    
    //mEps = Rand::randFloat(mFormationRadius/3.0f,mFormationRadius);
    mKernel->setArg(ARG_EPS, mEps);
    
    // update flags // TODO: cleanup
    mFlags = PARTICLE_FLAGS_NONE;
    mFlags |= PARTICLE_FLAGS_SHOW_MASS;//PARTICLE_FLAGS_SHOW_SPEED;//PARTICLE_FLAGS_SHOW_DARK;
    if( mUseInvSquareCalc ) mFlags |= PARTICLE_FLAGS_INVSQR;
        
    mKernel->setArg(ARG_FLAGS, mFlags);
    
    mKernel->run1D(mNumParticles);
	
    mKernel->setArg(ARG_POS_IN, mSwap ? mClBufPos1.getCLMem() : mClBufPos0.getCLMem());
    mKernel->setArg(ARG_POS_OUT, mSwap ? mClBufPos0.getCLMem() : mClBufPos1.getCLMem());
    mKernel->setArg(ARG_VEL_IN, mSwap ? mClBufVel1.getCLMem() : mClBufVel0.getCLMem());
    mKernel->setArg(ARG_VEL_OUT, mSwap ? mClBufVel0.getCLMem() : mClBufVel1.getCLMem());
    
    mKernel->setArg(ARG_COLOR, mClBufColor.getCLMem());

#else
    int nparticle = 8192; /* MUST be a nice power of two for simplicity */
    int nstep = 100;
    int nburst = 20; /* MUST divide the value of nstep without remainder */
    int nthread = 64; /* chosen for ATI Radeon HD 5870 */
    
    float dt1 = 0.0001;
    float eps = 0.0001;
    
    mKernel->setArg(ARG_POS_IN, mSwap ? mClBufPos1.getCLMem() : mClBufPos0.getCLMem());
    mKernel->setArg(ARG_POS_OUT, mSwap ? mClBufPos0.getCLMem() : mClBufPos1.getCLMem());
    mKernel->setArg(ARG_VEL, mClBufVel0.getCLMem());
    mKernel->setLocalArg(ARG_PBLOCK,nthread*sizeof(cl_float4));
    
    clmsync(stdgpu,0,pos1,CL_MEM_DEVICE|CL_EVENT_NOWAIT);
    clmsync(stdgpu,0,vel,CL_MEM_DEVICE|CL_EVENT_NOWAIT);
    for(int step=0; step<nstep; step+=nburst) {
        
        for(int burst=0; burst<nburst; burst+=2) {
            
            clarg_set_global(stdgpu,krn,2,pos1);
            clarg_set_global(stdgpu,krn,3,pos2);
            clfork(stdgpu,0,krn,&ndr,CL_EVENT_NOWAIT);
            
            clarg_set_global(stdgpu,krn,2,pos2);
            clarg_set_global(stdgpu,krn,3,pos1);
            clfork(stdgpu,0,krn,&ndr,CL_EVENT_NOWAIT);
            
        }
        
        clmsync(stdgpu,0,pos1,CL_MEM_HOST|CL_EVENT_NOWAIT);
        
        clwait(stdgpu,0,CL_KERNEL_EVENT|CL_MEM_EVENT|CL_EVENT_RELEASE);
        
    }
#endif
    
    mSwap = !mSwap;
    
	//mKernelUpdate->setArg(2, mMousePos);
	//mKernelUpdate->setArg(3, mDimensions);
	//mKernelUpdate->run1D(kNumParticles);
    
    updateCamera(dt);
    
    //updateHud();
    
    // debug info
    char buf[256];
    snprintf(buf, 256, "particles: %d", mNumParticles);
    mApp->getInfoPanel().addLine(buf, Color(0.75f, 0.5f, 0.5f));
    snprintf(buf, 256, "massives: %d", mStep);
    mApp->getInfoPanel().addLine(buf, Color(0.75f, 0.5f, 0.5f));
    
    Scene::update(dt);
}

//
// handleKeyDown
//
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
    AudioInput& audioInput = mApp->getAudioInput();
    std::shared_ptr<float> fftDataRef = audioInput.getFftDataRef();
    
    unsigned int bandCount = audioInput.getFftBandCount();
    float* fftBuffer = fftDataRef.get();
    
    if( fftBuffer )
    {
        mClBufFft.write( fftBuffer, 0, sizeof(cl_float) * bandCount );
        mKernel->setArg( ARG_FFT, mClBufFft.getCLMem() );
    }
}

void Graviton::updateCamera(const double dt)
{
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

//
// MARK: Render
//
void Graviton::draw()
{
    glPushMatrix();
    
    //gl::enableDepthWrite( false );
	//gl::enableDepthRead( false );
	//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	/*
    glColor3f(1.0f, 1.0f, 1.0f);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[0]);
#ifdef USE_OPENGL_CONTEXT
	mOpenCl.finish();
#else	
	mOpencl.readBuffer(sizeof(Vec2f) * NUM_PARTICLES, mClMemPosVBO, mParticlesPos);
	glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(Vec2f) * NUM_PARTICLES, mParticlesPos);
#endif	
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, 0);
	glDrawArrays(GL_POINTS, 0, kNumParticles);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    */
    
    if( mUseMotionBlur )
    {
        mMotionBlurRenderer.draw();
    }
    else
    {
        drawParticles();
    }
    
    glPopMatrix();
}

void Graviton::preRender() 
{
    switch( mCamType )
    {
        case CAM_SPIRAL:
            gl::setMatrices( mCam );
            break;
            
        case CAM_ORBITER:
        {
            Orbiter* orbiterScene = static_cast<Orbiter*>(mApp->getScene(0));
            
            if( orbiterScene && orbiterScene->isActive() )
            {
                gl::setMatrices( orbiterScene->getCamera() );
            }
            else
            {
                gl::setMatrices( mApp->getMayaCam() );
            }
        }
            break;
            
        default:
            gl::setMatrices( mApp->getMayaCam() );
    }
    
	if(mUseImageForPoints) 
    {
        if( mScalePointsByDistance )
        {
            glPointSize(mPointSize);
            float quadratic[] =  { 0.0f, 0.0f, 0.00001f };
            float sizes[] = { 3.0f, mPointSize };
            glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, sizes);
            glDisable(GL_POINT_SPRITE);
            glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
            glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, sizes[1] );
            glPointParameterfARB( GL_POINT_SIZE_MIN_ARB, sizes[0] );
            glPointParameterfARB( GL_POINT_FADE_THRESHOLD_SIZE_ARB, 60.0f );
            glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic );
            glTexEnvf( GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE );
            glEnable( GL_POINT_SPRITE_ARB );
        }
        else
        {
            glDisable(GL_POINT_SPRITE_ARB);
            glEnable(GL_POINT_SPRITE);
            glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
            glPointSize(mPointSize);
        }
	} 
    else 
    {
		glDisable(GL_POINT_SPRITE);
	}
	
	if(mAdditiveBlending) 
    {
        gl::enableAdditiveBlending();
	} 
    else 
    {
		gl::enableAlphaBlending();
	}
	
	if(mEnableLineSmoothing) 
    {
		glEnable(GL_LINE_SMOOTH);
	}
    else 
    {
		glDisable(GL_LINE_SMOOTH);
	}
	
	if(mEnablePointSmoothing) 
    {
		glEnable(GL_POINT_SMOOTH);
	} 
    else 
    {
		glDisable(GL_POINT_SMOOTH);
	}
    
	glLineWidth(mLineWidth);
    
    gl::disableDepthWrite();
    
}


void Graviton::drawParticles() 
{
    preRender();
    
    mOpenCl.flush();
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[0]);
    glVertexPointer(4, GL_FLOAT, 0, 0);
    
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[1]);
    glColorPointer(4, GL_FLOAT, 0, 0);
    
    if(mUseImageForPoints) 
    {
        glEnable(GL_TEXTURE_2D);
        mParticleTexture.bind();
    }
    
    glDrawArrays(GL_POINTS, 0, mNumParticles);
    
    if(mUseImageForPoints) 
    {
        mParticleTexture.unbind();
        //glDisable(GL_TEXTURE_2D);
    }
    
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glDisableClientState(GL_COLOR_ARRAY);
    
/*    
    if(mDoDrawNodes) 
    {
        drawNodes();
    }
    
    mOpenCl.flush();
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[0]);
    glVertexPointer(2, GL_FLOAT, 0, 0);
    
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[1]);
    glColorPointer(4, GL_FLOAT, 0, 0);
    
    if(mDoDrawLines) 
    {
        glDrawElements(GL_LINES, mNumParticles * kMaxTrailLength, GL_UNSIGNED_INT, mIndices);
        //glDrawArrays(GL_LINES, 0, kMaxParticles);
    }
    
    if(mDoDrawPoints)
    {
        if(mUseImageForPoints) 
        {
            //glEnable(GL_TEXTURE_2D);
            mParticleTexture.bind();
        }
        glDrawArrays(GL_POINTS, 0, mNumParticles);
            
        if(mUseImageForPoints) 
        {
            mParticleTexture.unbind();
            //glDisable(GL_TEXTURE_2D);
        }
    }
    
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    
    glDisableClientState(GL_COLOR_ARRAY);
    
    glPopMatrix();	
 */
}

void Graviton::drawDebug()
{
    if( NODE_FORMATION_NONE != mGravityNodeFormation )
    {
        gl::pushMatrices();
    
        CameraOrtho textCam(0.0f, app::getWindowWidth(), app::getWindowHeight(), 0.0f, 0.0f, 10.f);
        gl::setMatrices(textCam);
    
    
        for( int i = 0; i < mNumNodes; ++i )
        {
            Vec3f worldCoords( mPosAndMass[i].x, mPosAndMass[i].y, mPosAndMass[i].z );
            Vec2f textCoords = getCamera().worldToScreen(worldCoords, mApp->getWindowWidth(), mApp->getWindowHeight());
            
            gl::drawString(toString(mGravityNodes[i].mMass),textCoords,ColorAf(1.0,1.0,1.0,0.4));
        }
    }
    
    gl::popMatrices();
}
