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
: Scene("Magneto")
{
}

Magnetosphere::~Magnetosphere()
{
}

void Magnetosphere::setup()
{
    // init params
    mTimeStep = 0.00075f;
    
    mUseInvSquareCalc = false;
    mFlags = PARTICLE_FLAGS_NONE;
        
    mAdditiveBlending = true;
    mEnableLineSmoothing = false;
    mEnablePointSmoothing = false;
    mUseImageForPoints = true;
    mScalePointsByDistance = false;
    mPointSize = 2.0f;
    mLineWidth = 1.0f;
    mParticleAlpha = 1.0f;
    
    mDamping = 1.0f;
    //mEps = 0.01f;//mFormationRadius * 0.5f;
    
    mDimensions.x = mApp->getViewportWidth();
    mDimensions.y = mApp->getViewportHeight();
    
    initOpenCl();
    
    mParticleTexture = gl::Texture( loadImage( app::loadResource( RES_GLITTER ) ) );
    mParticleTexture.setWrap( GL_REPEAT, GL_REPEAT );
    
    reset();
    
    mMotionBlurRenderer.setup( mApp->getWindowSize(), boost::bind( &Magnetosphere::drawParticles, this ) );
}

void Magnetosphere::initOpenCl()
{
    mOpenCl.setupFromOpenGL();
    
    const size_t posBufSize = kMaxParticles * sizeof(float2) * kParticleTrailSize;
    const size_t colBufSize = kMaxParticles * sizeof(float4) * kParticleTrailSize;

    // init VBO
    //
    glGenBuffersARB(2, mVbo); // 2 VBOs, color and position
	
    // use VBO for position
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[0]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, posBufSize, mPosBuffer, GL_STREAM_COPY_ARB);
	glVertexPointer(2, GL_FLOAT, 0, 0);
	
    // use VBO for color
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[1]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, colBufSize, mColorBuffer, GL_STREAM_COPY_ARB);
	glColorPointer(4, GL_FLOAT, 0, 0);
	
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    
    // init CL kernel
    //
    fs::path clPath = App::getResourcePath("Particle.cl");
	mOpenCl.loadProgramFromFile(clPath.string());
	mKernel = mOpenCl.loadKernel("magnetoParticle");
	
	// init CL buffers
    //
    mClBufParticles.initBuffer(sizeof(tParticle) * kMaxParticles, CL_MEM_READ_WRITE);
    mClBufNodes.initBuffer(sizeof(tNode) * kMaxNodes, CL_MEM_READ_ONLY, NULL);
    
    mClBufPos.initFromGLObject(mVbo[0]);
	mClBufColor.initFromGLObject(mVbo[1]);
    mClBufFft.initBuffer( sizeof(cl_float)*kFftBands, CL_MEM_READ_WRITE );
}

void Magnetosphere::reset()
{
    initParticles();
    
    const size_t posBufSize = kMaxParticles * sizeof(float2) * kParticleTrailSize;
    const size_t colBufSize = kMaxParticles * sizeof(float4) * kParticleTrailSize;

    // recreate the buffers (afaik there's no leak here)
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[0]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, posBufSize, mPosBuffer, GL_STREAM_COPY_ARB);
	glVertexPointer(2, GL_FLOAT, 0, 0);
	
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[1]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, colBufSize, mColorBuffer, GL_STREAM_COPY_ARB);
	glColorPointer(4, GL_FLOAT, 0, 0);
    
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    mClBufParticles.write( mParticles, 0, sizeof(tParticle) * kMaxParticles );
    mClBufNodes.write( mNodes, 0, sizeof(tNode) * kMaxNodes );
    
    mKernel->setArg(ARG_PARTICLES, mClBufParticles.getCLMem());
    mKernel->setArg(ARG_NODES, mClBufNodes.getCLMem());
    mKernel->setArg(ARG_POS, mClBufPos.getCLMem());
    mKernel->setArg(ARG_COLOR, mClBufColor.getCLMem());
    mKernel->setArg(ARG_FFT, mClBufFft.getCLMem());
    mKernel->setArg(ARG_NUM_PARTICLES, mNumParticles);
    mKernel->setArg(ARG_DIMENSIONS, mDimensions);
}

void Magnetosphere::resize()
{
    mDimensions.x = mApp->getViewportWidth();
    mDimensions.y = mApp->getViewportHeight();
    mKernel->setArg(ARG_DIMENSIONS, mDimensions);
    mMotionBlurRenderer.resize(mApp->getWindowSize());
}

void Magnetosphere::setupParams(params::InterfaceGl& params)
{
    params.addText( "magneto", "label=`Magneto`" );
    params.addParam("Inv Square", &mUseInvSquareCalc );
    params.addParam("Time Step_", &mTimeStep, "step=0.0001 min=0.0 max=1.0" );
    params.addParam("Damping", &mDamping, "min=0.0 step=0.0001");
    
    params.addParam("Point Size", &mPointSize, "");
    params.addParam("Point Smoothing", &mEnablePointSmoothing, "");
    params.addParam("Point Sprites", &mUseImageForPoints, "");
    params.addParam("Point Scaling", &mScalePointsByDistance, "");
    params.addParam("Additive Blending", &mAdditiveBlending, "");
    params.addParam("Motion Blur", &mUseMotionBlur);
    params.addParam("Alpha", &mParticleAlpha, "min=0.0 max=1.0 step=0.001");

}

void Magnetosphere::initParticles()
{
    mNumParticles = kMaxParticles;
 
    for (int i=0; i < kMaxParticles; ++i) 
    {
		tParticle &p = mParticles[i];
		p.mVel.set(0.0f,0.0f);
		p.mMass = Rand::randFloat(0.5f, 1.0f);
        p.mLife = Rand::randFloat();
		mPosBuffer[i].x = Rand::randFloat(mApp->getViewportWidth());
        mPosBuffer[i].y = Rand::randFloat(mApp->getViewportHeight());
	}
    
    mNumNodes = kMaxNodes;
    for (int i=0; i < kMaxNodes; ++i) 
    {
		tNode &node = mNodes[i];
		node.mPos.x = Rand::randFloat(mApp->getViewportWidth());
        node.mPos.y = Rand::randFloat(mApp->getViewportHeight());
        node.mMass = Rand::randFloat(0.5f, 1.0f);
        node.mCharge = Rand::randFloat(-1.0f, 1.0f);
	}
}

void Magnetosphere::update(double dt)
{
    updateAudioResponse();
    //updateNodes();
    
    mKernel->setArg(ARG_DT, mTimeStep);
    mKernel->setArg(ARG_NUM_PARTICLES, mNumParticles);
    mKernel->setArg(ARG_DAMPING, mDamping);
    mKernel->setArg(ARG_ALPHA, mParticleAlpha);
    mKernel->setArg(ARG_COLOR, mClBufColor.getCLMem());
    mKernel->setArg(ARG_NUM_NODES, mNumNodes);
    //mKernel->setArg(ARG_NODES, mClBufNodes.getCLMem());
    mKernel->setArg(ARG_MOUSEPOS, mMousePos);
	
	// update flags // TODO: cleanup
    mFlags = PARTICLE_FLAGS_SHOW_MASS;
    //mFlags |= PARTICLE_FLAGS_SHOW_MASS;//PARTICLE_FLAGS_SHOW_SPEED;//PARTICLE_FLAGS_SHOW_DARK;
    if( mUseInvSquareCalc ) {
        mFlags |= PARTICLE_FLAGS_INVSQR;
    }
    mKernel->setArg(ARG_FLAGS, mFlags);
    
    mKernel->run1D(mNumParticles);
    
    //updateHud();
    
    // debug info
    char buf[256];
    snprintf(buf, 256, "m/particles: %d", mNumParticles);
    mApp->getInfoPanel().addLine(buf, Color(0.75f, 0.5f, 0.5f));
    
    Scene::update(dt);
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
    
    unsigned int bandCount = audioInput.getFftBandCount();
    float* fftBuffer = fftDataRef.get();
 
    if( fftBuffer )
    {
        mClBufFft.write( fftBuffer, 0, sizeof(cl_float) * bandCount );
        mKernel->setArg( ARG_FFT, mClBufFft.getCLMem() );
    }
}

//
// MARK: Render
//
void Magnetosphere::draw()
{
    glPushMatrix();
    gl::setMatricesWindow( mApp->getWindowSize() );

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

//
//
//
/*
void Magnetosphere::updateNode(clNode &n, int i) 
{
	float t;
    //	i = 1;
	int f = mRenderDimensions.x * 0.05;
	int num = 512;//fft.getNumAverages();//TODO:audio
	//int num2 = num/4;
	
	//t = fft.getPeaks()[i % num];
    t = Rand::randFloat(0.45f,0.55f);
	n.spread = lerp<float>(mSpreadMin, mSpreadMax, t * t) * f;
	
	//t = fft.getPeaks()[(i+num2) % num];
    t = Rand::randFloat(0.45f,0.55f);
	n.attractForce = lerp<float>(mNodeAttractMin, mNodeAttractMax, t * t) * f;
	
	//t = fft.getPeaks()[(i+num2*2) % num];
    t = Rand::randFloat(0.45f,0.55f);
	n.waveFreq = lerp<float>(mWaveFreqMin, mWaveFreqMax, t * t);
	
	//t = fft.getAverages()[(i+num2*3) % num];
    t = Rand::randFloat(0.45f,0.55f);
	n.waveAmp = lerp<float>(mWaveAmpMin, mWaveAmpMax, t * t * t) * f;
}



void Magnetosphere::updateNodes() 
{
	mNumNodes = mIsMousePressed ? 1 : 0;
	if(mNumNodes > kMagnetoMaxNodes) mNumNodes = kMagnetoMaxNodes;
    //TODO: audio
    float value = Rand::randFloat();//fft.avgPower;
	mCenterDeviation = lerp<float>(mCenterDeviationMin, mCenterDeviationMax, value);
	
	for(int i=0; i<mNumNodes; i++) 
    {		
		clNode &n = mNodes[i];
		n.pos.x = mMousePos.x;
		n.pos.y = mMousePos.y;
		updateNode(n, i * 2);
    }
	
	float sideDist = 0.2f;
	switch(mNumNodes) 
    {
		case 0:
			while(mNumNodes<=1) 
            {
				clNode &n = mNodes[mNumNodes];
				n.pos.x = mRenderDimensions.x * ((mNumNodes == 0 ? sideDist : 1.0f - sideDist) + Rand::randFloat() * mCenterDeviation/2);
				n.pos.y = mRenderDimensions.y * (0.5 + Rand::randFloat() * mCenterDeviation/2.0f);
				updateNode(n, mNumNodes);
				n.attractForce *= 0.5f;
				mNumNodes++;
			}
			break;
			
		case 1:
            //			Node &n = nodes[numNodes];
            //			float sideDist = 0.2f;
            ////			n.pos.x = renderDimensions.x * (0.5 + ofRandomf() * centerDeviation/2);
            //			n.pos.x = renderDimensions.x * ((nodes[0].pos.x > renderDimensions.x/2 ? sideDist : 1.0f - sideDist) + ofRandomf() * centerDeviation/2);
            //			n.pos.y = renderDimensions.y * (0.5 + ofRandomf() * centerDeviation/2);
            //			updateNode(n, numNodes);
            //			n.attractForce *= 0.2f;
            //			numNodes++;
			while(mNumNodes<=2) 
            {
				clNode &n = mNodes[mNumNodes];
				n.pos.x = mRenderDimensions.x * ((mNumNodes == 1 ? sideDist : 1.0f - sideDist) + Rand::randFloat() * mCenterDeviation/2.0f);
				n.pos.y = mRenderDimensions.y * (0.5 + Rand::randFloat() * mCenterDeviation/2.0f);
				updateNode(n, mNumNodes);
				n.attractForce *= 0.5f;
				mNumNodes++;
			}
			break;
	}
    
    //	openCL.writeBuffer(sizeof(Node) * numNodes, clBufNodes, nodes, false);		// TODO, test smaller buffer write
	mClBufNodes.write(mNodes, 0, sizeof(clNode) * mNumNodes);
	mKernel->setArg(ARG_NODES, mClBufNodes.getCLMem());
	mKernel->setArg(ARG_NUM_NODES, mNumNodes);
	
	mNodesNeedUpdating = false;
}

void Magnetosphere::drawNodes() 
{
    glEnable( GL_TEXTURE_2D );
	glColor4f(mColor.x * mNodeBrightness, mColor.y * mNodeBrightness, mColor.z * mNodeBrightness, 1.0f);
    //gl::enableDepthRead();
    //gl::enableDepthWrite();
    //gl::enableAdditiveBlending();
    //gl::enableAdditiveBlending();
    gl::enableDepthWrite( false );
    mParticleTexture.bind();
    glBegin( GL_QUADS );
    
	for(int i=0; i<mNumNodes; i++) 
    {
		clNode &n = mNodes[i];
		
        //        ofCircle(n.pos.x, n.pos.y, nodeRadius);
		float s = mNodeRadius * 2.0f;//ofMap(n.attractForce, nodeAttractMin, nodeAttractMax, 0, nodeRadius * 2.0f);
		//imageParticle.draw(n.pos.x, n.pos.y, s, s);
        glTexCoord2f( 0, 0 );
        glVertex2f( n.pos.x - s, n.pos.y - s );
        
        glTexCoord2f( 1, 0 );
        glVertex2f( n.pos.x + s, n.pos.y - s );
        
        glTexCoord2f( 1, 1 );
        glVertex2f( n.pos.x + s, n.pos.y + s );
        
        glTexCoord2f( 0, 1 );
        glVertex2f( n.pos.x - s, n.pos.y + s );
        
        
		//		ofDrawBitmapString(ofToString(t.ID), n.x * ofGetWidth(), n.y * ofGetHeight());
    }
	glEnd();
    mParticleTexture.unbind();
    glDisable( GL_TEXTURE_2D );
}
*/
void Magnetosphere::preRender() 
{
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

void Magnetosphere::drawParticles() 
{
    preRender();
    
    mOpenCl.flush();
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    // bind particle position VBO
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[0]);
    glVertexPointer(2, GL_FLOAT, 0, 0);
    
    // bind particle color VBO
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[1]);
    glColorPointer(4, GL_FLOAT, 0, 0);
    
    //TODO: trails/lines
    //glDrawElements(GL_LINES, mNumParticles * kMagnetoMaxTrailLength, GL_UNSIGNED_INT, mIndices);
        //glDrawArrays(GL_LINES, 0, kMaxParticles);
    //}
    
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
    
    glPopMatrix();	
}

void Magnetosphere::drawDebug()
{
}


/*
void Magnetosphere::drawStrokes() 
{
	glColor4f(mColor.x * mTrailBrightness, mColor.y * mTrailBrightness, mColor.z * mTrailBrightness, 1.0f);
	//ofNoFill();
	for(int i=0; i<kMagnetoMaxNodes; i++) 
    {
		//ofLine(oldNodes[i].x, oldNodes[i].y, nodes[i].pos.x, nodes[i].pos.y);
		mOldNodes[i].id = i;
		mOldNodes[i].add(mNodes[i].pos.x, mNodes[i].pos.y);
		mOldNodes[i].draw(mTrailAudioDistortMin, mTrailAudioDistortMax, mTrailWaveFreqMin, mTrailWaveFreqMax, mTrailDistanceAffectsWave, mTrailBrightness, mColor, mRenderDimensions);
        
        //console() << i << ": " << mNodes[i].pos.x << ", " << mNodes[i].pos.y << std::endl;
	}
	//ofFill();
}


void Magnetosphere::drawOffscreen() 
{
    Area viewport = gl::getViewport();
    
	mFboNew.bindFramebuffer();			// draw new frame on a clear transparent background
    {
        gl::setMatricesWindow( mFboNew.getSize(), false );
        gl::setViewport( mFboNew.getBounds() );
        gl::clear( ColorA(0,0,0,0.0f) );
        
        drawParticles();
        drawStrokes();
    }
    mFboNew.unbindFramebuffer();
	
    mFboComp.bindFramebuffer();			// composite prev blurred version with new frame on clear transparent background
    {
        gl::setMatricesWindow( mFboComp.getSize(), false );
        gl::setViewport( mFboComp.getBounds() );
        gl::clear( ColorA(0,0,0,1.0f) );
        gl::enableAdditiveBlending();
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_ONE, GL_ONE);
        
		float c = 1.0f - mFadeSpeed;
        glColor4f(c, c, c, 1.0f);
        
//        gl::clear( Color::black() );
        //mFboBlur.bindTexture(0); // use rendered scene as texture
//        gl::pushMatrices();
//        gl::setMatricesWindow(512, 512, false);
        //gl::drawSolidRect( mFboComp.getBounds() );
//        gl::popMatrices();
        //mFboBlur.unbindTexture();
//        mFboTemporary.unbindFramebuffer();
        
        gl::draw( mFboBlur.getTexture(0), mFboComp.getBounds() );
		
        glColor3f(1, 1, 1);
        gl::draw( mFboNew.getTexture(0), mFboComp.getBounds() );
        //mFboNew.bindTexture(0); // use rendered scene as texture
        //gl::drawSolidRect( mFboComp.getBounds() );
        //mFboNew.unbindTexture();

    }
    mFboComp.unbindFramebuffer();
	
	
    mFboBlur.bindFramebuffer();
    {
        gl::setMatricesWindow( mFboBlur.getSize(), false );
        gl::setViewport( mFboBlur.getBounds() );
		gl::clear( ColorA(0,0,0,1.0f) );
        if(mDoBlur) 
        {
            mBlurShader.bind();
            float invWidth = 1.0f/mFboBlur.getWidth();
            float invHeight = 1.0f/mFboBlur.getHeight();
            float i = mBlurAmount;
            mBlurShader.uniform("amountX", 1 * invWidth * i);
            mBlurShader.uniform("amountY", 1 * invHeight * i);
        }
		
        gl::draw( mFboComp.getTexture(0), mFboBlur.getBounds() );
        //mFboComp.bindTexture(0); // use rendered scene as texture
        //gl::drawSolidRect( mFboBlur.getBounds() );
        //mFboComp.unbindTexture();
        
        if(mDoBlur) 
            mBlurShader.unbind();
    }
    mFboBlur.unbindFramebuffer();
	
	
	
	
    /*	
     
     fboNew.begin();			// draw new frame on a clear transparent background
     {
     //        fboNew.clear();
     //        drawParticles();
     glEnable(GL_BLEND);
     if(fadeSpeed) {
     glColor4f(0, 0, 0, fadeSpeed);
     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     ofRect(0, 0, fboNew.getWidth(), fboNew.getHeight());
     }
     
     glBlendFunc(GL_ONE, GL_ONE);
     fboBlur.draw(0, 0, fboNew.getWidth(), fboNew.getHeight());
     
     drawStrokes();
     }
     fboNew.end();
     
     fboBlur.begin();
     {
     glEnable(GL_BLEND);
     glBlendFunc(GL_ONE, GL_ONE);
     fboBlur.clear();
     if(doBlur) {
     float invWidth = 1.0f/fboBlur.getWidth();
     float invHeight = 1.0f/fboBlur.getHeight();
     fboBlur.bindAsTexture();
     blurShader.setShaderActive(true);
     blurShader.setUniform("texture", 0);
     blurShader.setUniform("blurCenterWeight", blurCenterWeight);
     float invBlurAmount = 1.0/blurIterations;
     for(int i=0; i<blurIterations; i++) {
     float curBlurAmount = ofMap(i, 0, blurIterations-1, 1, blurAmount);
     blurShader.setUniform("amountX", invWidth * curBlurAmount);
     blurShader.setUniform("amountY", invHeight * curBlurAmount);
     glColor3f(invBlurAmount, invBlurAmount, invBlurAmount);
     fboNew.draw(0, 0, fboBlur.getWidth(), fboBlur.getHeight());
     }
     blurShader.setShaderActive(false);
     } else {
     fboNew.draw(0, 0, fboBlur.getWidth(), fboBlur.getHeight());
     }
     }
     fboBlur.end();
     *
    
    gl::setViewport( viewport );
}
 */
