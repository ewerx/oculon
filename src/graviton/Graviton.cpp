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

using namespace ci;
using namespace ci::app;

#define PARTICLE_FLAGS_NONE         0x00
#define PARTICLE_FLAGS_INVSQR       0x02
#define PARTICLE_FLAGS_SHOW_DARK    0x04


Graviton::Graviton()
: Scene()
{
}

Graviton::~Graviton()
{
}

void Graviton::setup()
{
    mTimeStep = 0.0005f;
    
    mUseImageForPoints = false;
    mUseInvSquareCalc = true;
    mFlags = PARTICLE_FLAGS_NONE;
    
    mInitialFormation = FORMATION_GALAXY;
    mFormationRadius = 300;
    
	initOpenCl();
	
	glPointSize(1);
    
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
    params.addParam( "Inv Square", &mUseInvSquareCalc );
    params.addParam( "Time Step", &mTimeStep, "step=0.0001 min=0.0 max=1.0" );
    params.addParam( "Initial Formation", (int*)(&mInitialFormation), "min=0 max=4" );
    params.addParam( "Formation Radius", &mFormationRadius, "min=1.0" );
}

void Graviton::initParticles()
{
    mStep = kStep;
    mNumParticles = kNumParticles;
    
    const double r = mFormationRadius;
    
    for( size_t i = 0; i < kNumParticles; ++i )
    {
        double x = 0.0f;
        double y = 0.0f;
        double z = 0.0f;
        
        double rho = 0.0f;
        double theta = 0.0f;
        
        switch( mInitialFormation )
        {
            case FORMATION_SPHERE:
            {
                rho = Utils::randDouble() * (M_PI * 2.0);
                theta = Utils::randDouble() * (M_PI * 2.0);
                
                x = r * cos(rho) * sin(theta);
                y = r * sin(rho) * sin(theta);
                z = r * cos(theta);
            }
                break;
                
            case FORMATION_SPHERE_SHELL:
            {
                
            }
                break;
                
            case FORMATION_DISC:
            {
                rho = r * pow(Utils::randDouble(), 0.75);
                theta = Utils::randDouble() * (M_PI * 2.0);
                theta = (0.5 * cos(2.0 * theta) + theta - 1e-2 * rho);
                
                const float thickness = 1.0f;
                
                x = rho * cos(theta);
                y = rho * sin(theta);
                z = thickness * 2.0 * Utils::randDouble() - 1.0;
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
                const float maxThickness = r / 4.0f;
                const float thickness =  maxThickness * EaseOutInQuad()(1.0f - dist / r);
                
                z = thickness * (2.0 * Utils::randDouble() - 1.0);
            }
                break;
                
            default:
                break;
        }
        
        // pos
        mPosAndMass[i].x = x;
        mPosAndMass[i].y = y;
        mPosAndMass[i].z = z;
        // mass
        const float maxMass = (i < mStep) ? 50.0f : 1.0f;
        mPosAndMass[i].w = Rand::randFloat(1.0f,maxMass);
        
        // vel
        const double a = 1.0e0 * (rho <= 1e-1 ? 0.0 : rho);
        mVel[i].x = -a * sin(theta);
        mVel[i].y = a * cos(theta);
        mVel[i].z = 0.0f;
        // unused
        mVel[i].w = 0.0f;
        
        // color
        mColor[i].x = 1.0f;
        mColor[i].y = 1.0f;
        mColor[i].z = 1.0f;
        mColor[i].w = 1.0f;
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

}

void Graviton::reset()
{
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
    mKernel->setArg(ARG_COUNT, mNumParticles);
    mKernel->setArg(ARG_STEP, mStep);
#else
    //TODO
    mKernel->setArg(ARG_POS_IN, mSwap ? mClBufPos1.getCLMem() : mClBufPos0.getCLMem());
    mKernel->setArg(ARG_POS_OUT, mSwap ? mClBufPos0.getCLMem() : mClBufPos1.getCLMem());
    mKernel->setArg(ARG_VEL, mClBufVel0.getCLMem());
#endif
    
    mSwap = false;
}

void Graviton::resize()
{
}

void Graviton::update(double dt)
{
    Scene::update(dt);
    
	//mDimensions.x = mApp->getViewportWidth();
	//mDimensions.y = mApp->getViewportHeight();
    
    updateAudioResponse();

#if defined( FREEOCL_VERSION )
    //mAnimTime += mTimeSpeed;
    //mKernelUpdate->setArg(ARG_TIME, mAnimTime);

    mKernel->setArg(ARG_DT, mTimeStep);
    mKernel->setArg(ARG_COUNT, mNumParticles);
    mKernel->setArg(ARG_STEP, mStep);
    mKernel->setArg(ARG_DAMPING, mDamping);
    
    // update flags // TODO: cleanup
    mFlags = PARTICLE_FLAGS_NONE;
    mFlags |= PARTICLE_FLAGS_SHOW_DARK;
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
    
    //updateHud();
    
    // debug info
    char buf[256];
    snprintf(buf, 256, "particles: %d", mNumParticles);
    mApp->getInfoPanel().addLine(buf, Color(0.75f, 0.5f, 0.5f));
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
    
    //unsigned int bandCount = audioInput.getFftBandCount();
    //float* fftBuffer = fftDataRef.get();
    
    //int bodyIndex = 0;
    
    //TODO
}

void Graviton::draw()
{
    glPushMatrix();
    //gl::setMatricesWindow( mApp->getWindowSize() );
    
    
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
    
    drawParticles();
    
    glPopMatrix();
}



void Graviton::preRender() 
{
    /*
	if(mUseImageForPoints) 
    {
		glEnable(GL_POINT_SPRITE);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	} 
    else 
    {
		glDisable(GL_POINT_SPRITE);
	}
	
	if(mEnableBlending || mAdditiveBlending) 
    {
		if(mAdditiveBlending) 
            gl::enableAdditiveBlending();
		else 
            gl::enableAlphaBlending();
	} 
    else 
    {
		glDisable(GL_BLEND);
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
    
	
	glPointSize(mPointSize);
	glLineWidth(mLineWidth);
     */
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
        //glEnable(GL_TEXTURE_2D);
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
