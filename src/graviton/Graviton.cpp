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
    
    initParticles();
    initVbo();
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
    
    const double r = 1e2; // wha??
    
    const Vec2f center = mApp->getWindowSize() / 2.0f;
    
    for( size_t i = 0; i < kNumParticles; ++i )
    {
        const double rho = r * pow((double(rand()) / RAND_MAX), 0.75);
        double theta = (double(rand()) / RAND_MAX) * (M_PI * 2.0);
        theta = (0.5 * cos(2.0 * theta) + theta - 1e-2 * rho);
        const double z = 2.0 * (double(rand()) / RAND_MAX) - 1.0;
        const double c = cos(theta);
        const double s = sin(theta);
        mPosAndMass[i].s[0] = rho * c;
        mPosAndMass[i].s[1] = rho * s;
        mPosAndMass[i].s[2] = z;
        mPosAndMass[i].s[3] = 1.0;
        
        const double a = 1.0e0 * (rho <= 1e-1 ? 0.0 : rho);
        mVel[i].s[0] = -a * s;
        mVel[i].s[1] = a * c;
        mVel[i].s[2] = 0.0f;
        mVel[i].s[3] = 0.0f;
    }
}

void Graviton::initVbo()
{
	glGenBuffersARB(1, mVbo);
    
    const size_t size = sizeof(cl_float4) * kNumParticles;
	
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[0]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, mPosAndMass, GL_STREAM_COPY_ARB);
	glVertexPointer(4, GL_FLOAT, 0, 0);
	
	//glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[1]);
	//glBufferDataARB(GL_ARRAY_BUFFER_ARB, kMagnetoSizeofColBuffer, mColorBuffer, GL_STREAM_COPY_ARB);
	//glColorPointer(4, GL_FLOAT, 0, 0);
	
    
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

void Graviton::initOpenCl()
{
    mOpenCl.setupFromOpenGL();
    
    /*
	glGenBuffersARB(2, mVbo);
	
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[0]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, kSizeofPosBuffer, mPosBuffer, GL_STREAM_COPY_ARB);
	glVertexPointer(2, GL_FLOAT, 0, 0);
	
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[1]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, kSizeofColBuffer, mColorBuffer, GL_STREAM_COPY_ARB);
	glColorPointer(4, GL_FLOAT, 0, 0);
	
    
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    */
    fs::path clPath = App::getResourcePath("Nbody.cl");
	mOpenCl.loadProgramFromFile(clPath.string());
	mKernel = mOpenCl.loadKernel("gravity");
	
    //mClBufPos0.initBuffer( sizeof(cl_float4) * kNumParticles, CL_MEM_READ_WRITE, mPosAndMass );
    mClBufPos0.initFromGLObject(mVbo[0]);
    mClBufPos1.initBuffer( sizeof(cl_float4) * kNumParticles, CL_MEM_READ_ONLY );
    mClBufVel0.initBuffer( sizeof(cl_float4) * kNumParticles, CL_MEM_READ_WRITE, mVel );
    mClBufVel1.initBuffer( sizeof(cl_float4) * kNumParticles, CL_MEM_READ_ONLY );
    
    
	mKernel->setArg(ARG_POS_IN, mClBufPos0.getCLMem());
    mKernel->setArg(ARG_POS_OUT, mClBufPos1.getCLMem());
    mKernel->setArg(ARG_VEL_IN, mClBufVel0.getCLMem());
    mKernel->setArg(ARG_VEL_OUT, mClBufVel1.getCLMem());
    mKernel->setArg(ARG_COUNT, mNumParticles);
    mKernel->setArg(ARG_STEP, mStep);
    
    mSwap = true;
    
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
    
    //mAnimTime += mTimeSpeed;
    //mKernelUpdate->setArg(ARG_TIME, mAnimTime);

    mKernel->setArg(ARG_COUNT, mNumParticles);
    mKernel->setArg(ARG_STEP, mStep);
    
    mKernel->run1D(mNumParticles);
	
    
    //std::swap(mClBufPos0, mClBufPos1);
    //std::swap(mClBufVel0, mClBufVel1);
    mKernel->setArg(ARG_POS_IN, mSwap ? mClBufPos1.getCLMem() : mClBufPos0.getCLMem());
    mKernel->setArg(ARG_POS_OUT, mSwap ? mClBufPos0.getCLMem() : mClBufPos1.getCLMem());
    mKernel->setArg(ARG_VEL_IN, mSwap ? mClBufVel1.getCLMem() : mClBufVel0.getCLMem());
    mKernel->setArg(ARG_VEL_OUT, mSwap ? mClBufVel0.getCLMem() : mClBufVel1.getCLMem());

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
