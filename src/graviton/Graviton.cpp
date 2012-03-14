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
#include "Utils.h"

using namespace ci;
using namespace ci::app;




Graviton::Graviton()
: Scene()
{
}

Graviton::~Graviton()
{
}

void Graviton::setup()
{
    mTimeStep = 0.01f;
    
    //initParticles();
	initOpenCl();
	
	glPointSize(1);
    
    //mParticleTexture = gl::Texture( loadImage( app::loadResource( RES_GLITTER ) ) );
    //mParticleTexture.setWrap( GL_REPEAT, GL_REPEAT );
    
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

void Graviton::initParticles()
{
    mStep = kStep;
    mNumParticles = kNumParticles;
    
    // position particles randomly inside a sphere
    //const double r = 1e2;
    
    const Vec2f center = mApp->getWindowSize() / 2.0f;
    
    for( size_t i = 0; i < kNumParticles; ++i )
    {
        const double r = Rand::randFloat(100.0f, 500.0f);
        
        //const double rho = radius * pow(Utils::randDouble(), 0.75);
        const double rho = Utils::randDouble() * (M_PI * 2.0);
        double theta = Utils::randDouble() * (M_PI * 2.0);
        //theta = (0.5 * cos(2.0 * theta) + theta - 1e-2 * rho);
        
        const double c = cos(theta);
        const double s = sin(theta);
        
        const double x = r * cos(rho) * sin(theta);
        const double y = r * sin(rho) * sin(theta);
        // sphere
        const double z = r * cos(theta);
        // disc
        //const double z = 2.0 * (double(rand()) / RAND_MAX) - 1.0;
        
        mPosAndMass[i].s[0] = x;//rho * c;
        mPosAndMass[i].s[1] = y;//rho * s;
        mPosAndMass[i].s[2] = z;
        mPosAndMass[i].s[3] = Rand::randFloat(1.0f,100000.0f);
        
        const double a = 1.0e0 * (rho <= 1e-1 ? 0.0 : rho);
        mVel[i].s[0] = -a * s;
        mVel[i].s[1] = a * c;
        mVel[i].s[2] = 0.0f;
        mVel[i].s[3] = 0.0f;
    }
}

void Graviton::initOpenCl()
{
    mOpenCl.setupFromOpenGL();
    
    const size_t size = sizeof(cl_float4) * kNumParticles;
    
    // init VBO
    //
    glGenBuffersARB(1, mVbo); // 1 VBO
    
    // use VBO instead of CL buffer for positions to render directly
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[0]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, mPosAndMass, GL_STREAM_COPY_ARB);
	glVertexPointer(4, GL_FLOAT, 0, 0);
	
	//glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[1]);
	//glBufferDataARB(GL_ARRAY_BUFFER_ARB, kMagnetoSizeofColBuffer, mColorBuffer, GL_STREAM_COPY_ARB);
	//glColorPointer(4, GL_FLOAT, 0, 0);
    
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
    
    
	
    
	//mClBufParticles.initBuffer(sizeof(clParticle) * kNumParticles, CL_MEM_READ_WRITE, mParticles);
    
    //mClBufParticles.initBuffer(sizeof(clParticle) * kNumParticles, CL_MEM_READ_WRITE, mParticles);
    //mClBufNodes.initBuffer(sizeof(clNode) * kMaxNodes, CL_MEM_READ_ONLY, NULL);
    
    
    //mClBufPosBuffer.initFromGLObject(mVbo[0]);
	//mClBufColorBuffer.initFromGLObject(mVbo[1]);
	
	//mKernelUpdate->setArg(ARG_PARTICLES, mClBufParticles.getCLMem());
    //mKernelUpdate->setArg(ARG_POS_BUFFER, mClBufPosBuffer.getCLMem());
    //mKernelUpdate->setArg(ARG_COLOR_BUFFER, mClBufColorBuffer.getCLMem());
}

void Graviton::reset()
{
    initParticles();
    
    const size_t size = sizeof(cl_float4) * kNumParticles;
	
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[0]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, mPosAndMass, GL_STREAM_COPY_ARB);
	glVertexPointer(4, GL_FLOAT, 0, 0);
	
	//glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[1]);
	//glBufferDataARB(GL_ARRAY_BUFFER_ARB, kMagnetoSizeofColBuffer, mColorBuffer, GL_STREAM_COPY_ARB);
	//glColorPointer(4, GL_FLOAT, 0, 0);
    
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    
    mClBufVel0.write( mVel, 0, sizeof(cl_float4) * kNumParticles );
    
#if defined( FREEOCL_VERSION )
    mKernel->setArg(ARG_POS_IN, mClBufPos0.getCLMem());
    mKernel->setArg(ARG_POS_OUT, mClBufPos1.getCLMem());
    mKernel->setArg(ARG_VEL_IN, mClBufVel0.getCLMem());
    mKernel->setArg(ARG_VEL_OUT, mClBufVel1.getCLMem());
    mKernel->setArg(ARG_COUNT, mNumParticles);
    mKernel->setArg(ARG_STEP, mStep);
#else
    //TODO
    mKernel->setArg(ARG_POS_IN, mSwap ? mClBufPos1.getCLMem() : mClBufPos0.getCLMem());
    mKernel->setArg(ARG_POS_OUT, mSwap ? mClBufPos0.getCLMem() : mClBufPos1.getCLMem());
    mKernel->setArg(ARG_VEL, mClBufVel0.getCLMem());
#endif
    
    mSwap = true;
}

void Graviton::resize()
{
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
    
    mKernel->run1D(mNumParticles);
	
    
    //std::swap(mClBufPos0, mClBufPos1);
    //std::swap(mClBufVel0, mClBufVel1);
    mKernel->setArg(ARG_POS_IN, mSwap ? mClBufPos1.getCLMem() : mClBufPos0.getCLMem());
    mKernel->setArg(ARG_POS_OUT, mSwap ? mClBufPos0.getCLMem() : mClBufPos1.getCLMem());
    mKernel->setArg(ARG_VEL_IN, mSwap ? mClBufVel1.getCLMem() : mClBufVel0.getCLMem());
    mKernel->setArg(ARG_VEL_OUT, mSwap ? mClBufVel0.getCLMem() : mClBufVel1.getCLMem());

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
    //glEnableClientState(GL_COLOR_ARRAY);
    
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[0]);
    glVertexPointer(4, GL_FLOAT, 0, 0);
    
    glDrawArrays(GL_POINTS, 0, mNumParticles);
    
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    
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
