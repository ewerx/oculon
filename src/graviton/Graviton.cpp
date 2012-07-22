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
#include "Interface.h"

using namespace ci;
using namespace ci::app;


Graviton::Graviton()
: Scene("graviton")
{
}

Graviton::~Graviton()
{
}

void Graviton::setup()
{
    Scene::setup();
    
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
    mPointSize = 1.0f;
    mParticleAlpha = 0.5f;
    mUseMotionBlur = false;
    
    mDamping = 1.0f;
    mGravity = 100.0f;
    mEps = 0.001f;//mFormationRadius * 0.5f;
    
    mNumNodes = 0;
    mGravityNodeFormation = NODE_FORMATION_NONE;
    
    // camera
    mCamRadius = mFormationRadius * 3.0f;
    mCamAngle = 0.0f;
    mCamMaxDistance = mFormationRadius * 3.0f;
    mCamLateralPosition = -mCamMaxDistance;
    mCamTarget = Vec3f::zero();
    mCamTurnRate = 0.25f;
    mCamTranslateRate = 1.0f;
    mCamType = CAM_SPLINE;
    
    
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
    
    mParticleTexture = gl::Texture( loadImage( app::loadResource( RES_PARTICLE_WHITE ) ) );
    mParticleTexture.setWrap( GL_REPEAT, GL_REPEAT );
    
    reset();
    
    mMotionBlurRenderer.setup( mApp->getWindowSize(), boost::bind( &Graviton::drawParticles, this ) );

}

void Graviton::setupDebugInterface()
{
    Scene::setupDebugInterface(); // add all interface params
    
    mDebugParams.setOptions("Time Step", "step=0.0001 min=-1.0 max=1.0" );
    mDebugParams.setOptions("Gravity", "step=0.1");
    mDebugParams.setOptions("Alpha", "min=0.0 max=1.0 step=0.001");
    
    mDebugParams.setOptions("Particle Formation", "min=0 max=3 enum='0 {Sphere}, 1 {Shell}, 2 {Disc}, 3 {Galaxy}'" );
    mDebugParams.setOptions("Node Formation", "min=0 max=2");
    mDebugParams.setOptions("Damping", "step=0.0001");
    mDebugParams.setOptions("EPS", "min=0.0 step=0.001");
    
    mDebugParams.setOptions("Cam Type", "min=0 max=3");
    mDebugParams.setOptions("Cam Radius", "min=1.0");
    mDebugParams.setOptions("Cam Distance", "min=0.0");
    mDebugParams.setOptions("Cam Turn Rate", "step=0.01");
    mDebugParams.setOptions("Cam Slide Rate", "step=0.1");
    
    mDebugParams.addSeparator();
    mDebugParams.addParam("Inv Square", &mUseInvSquareCalc );
    mDebugParams.addParam("Motion Blur", &mUseMotionBlur);
    mDebugParams.addParam("Point Smoothing", &mEnablePointSmoothing, "");
    mDebugParams.addParam("Point Sprites", &mUseImageForPoints, "");
    mDebugParams.addParam("Point Scaling", &mScalePointsByDistance, "");
    mDebugParams.addParam("Additive Blending", &mAdditiveBlending, "");
    
}

void Graviton::setupInterface()
{
    mInterface->addParam(CreateFloatParam( "Time Step", &mTimeStep )
                         .minValue(0.0f)
                         .maxValue(0.001f)
                         .oscReceiver(getName(), "timestep"));
                         //.oscReceiver("/1/fader1"));
                         //.oscSender("/1/fader1"));
    mInterface->addParam(CreateFloatParam( "Damping", &mDamping )
                         .oscReceiver(getName(), "damping"));
    mInterface->addParam(CreateFloatParam( "EPS", &mEps )
                         .minValue(0.0001)
                         .maxValue(1500)
                         .oscReceiver(getName(), "eps"));
    mInterface->addParam(CreateFloatParam( "Gravity", &mGravity )
                         .minValue(0.0f)
                         .maxValue(100.0f)
                         .oscReceiver(getName(), "gravity"));
                         //.oscReceiver("/1/fader2"));
    
    mInterface->gui()->addSeparator();
    mInterface->addEnum(CreateEnumParam( "Particle Formation", (int*)(&mInitialFormation) )
                         .maxValue(FORMATION_COUNT)
                         .oscReceiver(getName(), "pformation")
                         .isVertical());
    mInterface->addEnum(CreateEnumParam( "Node Formation", (int*)(&mGravityNodeFormation) )
                         .maxValue(NODE_FORMATION_COUNT)
                         .oscReceiver(getName(), "nformation")
                         .isVertical());
    mInterface->addParam(CreateFloatParam( "Formation Radius", &mFormationRadius )
                         .minValue(10.0f)
                         .maxValue(1000.0f)
                         .oscReceiver(getName(), "formradius"));
    
    mInterface->gui()->addSeparator();
    mInterface->addParam(CreateFloatParam( "Alpha", &mParticleAlpha )
                         .oscReceiver(getName(), "alpha"));
    mInterface->addParam(CreateFloatParam( "Point Size", &mPointSize )
                         .minValue(1.0f)
                         .maxValue(3.0f)
                         .oscReceiver(getName(), "psize"));
    
    mInterface->gui()->addColumn();
    mInterface->addEnum(CreateEnumParam( "Cam Type", (int*)(&mCamType) )
                         .maxValue(CAM_COUNT)
                         .oscReceiver(getName(), "camtype")
                         .isVertical());
    mInterface->addParam(CreateFloatParam( "Cam Radius", &mCamRadius )
                         .minValue(1.0f)
                         .maxValue(500.f)
                         .oscReceiver(getName(), "camradius"));
    mInterface->addParam(CreateFloatParam( "Cam Distance", &mCamMaxDistance )
                         .minValue(0.0f)
                         .maxValue(500.f));
    mInterface->addParam(CreateFloatParam( "Cam Turn Rate", &mCamTurnRate )
                         .minValue(0.0f)
                         .maxValue(5.0f)
                         .oscReceiver(getName(), "camturn"));
    mInterface->addParam(CreateFloatParam( "Cam Slide Rate", &mCamTranslateRate )
                         .minValue(0.0f)
                         .maxValue(5.0f)
                         .oscReceiver(getName(), "camspeed"));
    mInterface->addButton(CreateTriggerParam("Reset Spline", NULL)
                          .oscReceiver(mName,"resetspline"))->registerCallback( this, &Graviton::setupCameraSpline );    
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
    
    // random spline
    setupCameraSpline();
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
	
    // recreate the buffers (afaik there's no leak here)
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
}

void Graviton::resize()
{
    Scene::resize();
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
    //mFlags |= PARTICLE_FLAGS_SHOW_MASS;//PARTICLE_FLAGS_SHOW_SPEED;//PARTICLE_FLAGS_SHOW_DARK;
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
    
    updateCamera(dt);
    
    //updateHud();
    
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
    gl::setMatrices( ( mCamType == CAM_MAYA ) ? mApp->getMayaCam() : mCam );
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
        case CAM_SPLINE:
        case CAM_SPIRAL:
            gl::setMatrices( mCam );
            break;
            
        case CAM_ORBITER:
        {
            Orbiter* orbiterScene = static_cast<Orbiter*>(mApp->getScene(0));
            
            if( orbiterScene && orbiterScene->isRunning() )
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
            glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, mPointSize );
            glPointParameterfARB( GL_POINT_SIZE_MIN_ARB, mPointSize );
            glDisable(GL_POINT_SPRITE_ARB);
            glDisable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
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
}

void Graviton::drawDebug()
{
    Scene::drawDebug();
    
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
        
        gl::popMatrices();
    }
    
    drawCamSpline();
}
