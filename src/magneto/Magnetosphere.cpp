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

/*static*/ int Magnetosphere::sOldNodeHistory = 10;


Magnetosphere::Magnetosphere()
: Scene("Magneto")
{
}

Magnetosphere::~Magnetosphere()
{
}

void Magnetosphere::setup()
{
    
    mEnableBlending = true;
    mAdditiveBlending = true;
    
    mNodeRadius = 2.0f; //100
    mDoDrawNodes = true;
    
    mPointSize = 2.0f;//20
    mDoDrawPoints = true;
    mUseImageForPoints = true;
    
    mDoDrawLines = false;
    mLineWidth = 1.0f;//20
    
    mNumParticlesPower = 1;
    mNumParticles = 100;
    
    mTimeSpeed = 1.0f;
    mDieSpeed = 0.0f;
    mMassMin = 0.0f;
    mMomentum = 0.01f;
    mSpreadMin = 0.0f;
    mSpreadMax = 1.0f;
    mNodeAttractMin = 0.0f;
    mNodeAttractMax = 1.0f;
    mWaveAmpMin = 0.0f;
    mWaveAmpMax = 1.0f;
    mWaveFreqMin = 0.0f;
    mWaveFreqMax = 1.0f;
    mWavePosMult = 1.0f;//10
    mWaveVelMult = 1.0f;//10
    
    mColor.set(1.0f,1.0f,1.0f,1.0f);
    mColorTaper = 0.5f;
    
    mNodeBrightness = 1.0f;
    
    // trail
    mUseFbo = false;
    mDoBlur = false;
    //mOldNodeHistory = 1;
    mFboScaleDown = 1.0f;//8
    mBlurAmount = 0.0f;//50
    mTrailBrightness = 0.5f;
    mTrailAudioDistortMin = 0.0f;
    mTrailAudioDistortMax = 5.0f;//10
    mTrailWaveFreqMin = 0.0f;
    mTrailWaveFreqMax = 1.0f;
    mTrailDistanceAffectsWave = false;
    
    mFadeSpeed = 0.001f;
    
    gl::Fbo::Format format;
    //format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
    const int fboWidth = mApp->getWindowWidth();
    const int fboHeight = mApp->getWindowHeight();
    mFboNew = gl::Fbo( fboWidth, fboHeight, format );
    mFboComp = gl::Fbo( fboWidth, fboHeight, format );
    mFboBlur = gl::Fbo( fboWidth, fboHeight, format );
    
    mFboBlur.bindFramebuffer();
    {
        gl::setMatricesWindow( mFboNew.getSize(), false );
        gl::setViewport( mFboNew.getBounds() );
        gl::clear( ColorA(0,0,0,0) );
    }
    mFboBlur.unbindFramebuffer();
    
    /*
    for(int i=0; i < kMagnetoNumParticles; i++) 
    {
		clParticle &p = mParticles[i];
		p.vel.set(0, 0);
		p.mass = Rand::randFloat(0.5f, 1.0f);		
		mPosBuffer[i].set(Rand::randFloat(mApp->getViewportWidth()), Rand::randFloat(mApp->getViewportHeight()));
	}
    */
    //int rowCount = sqrtf(kMagnetoNumParticles);
	//int colCount = kMagnetoNumParticles / rowCount;
	
	for(int i=0; i<kMagnetoNumParticles; i++) 
    {
		mIndices[i*2]	= i;
		mIndices[i*2+1]	= i + kMagnetoNumParticles;
		
		clParticle &p = mParticles[i];
		p.vel.x = Rand::randFloat();
		p.vel.y = Rand::randFloat();
		
		//		p.homePos.x = Rand::randFloat();//(i%colCount) * 1.0f/rowCount;//;
		//		p.homePos.y = Rand::randFloat();//(i/colCount) * 1.0f/rowCount;//Rand::randFloat();
		
		p.mass = Rand::randFloat(mMassMin, 1.0f);
		
		p.life = Rand::randFloat();
		
		mPosBuffer[i].x = Rand::randFloat(mApp->getViewportWidth());
		mPosBuffer[i].y = Rand::randFloat(mApp->getViewportHeight());	
	}
    
	initOpenCl();
	
	glPointSize(1);
    
    mParticleTexture = gl::Texture( loadImage( app::loadResource( RES_GLITTER ) ) );
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

void Magnetosphere::initOpenCl()
{
    mOpenCl.setupFromOpenGL();
    
	glGenBuffersARB(2, mVbo);
	
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[0]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, kMagnetoSizeofPosBuffer, mPosBuffer, GL_STREAM_COPY_ARB);
	glVertexPointer(2, GL_FLOAT, 0, 0);
	
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[1]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, kMagnetoSizeofColBuffer, mColorBuffer, GL_STREAM_COPY_ARB);
	glColorPointer(4, GL_FLOAT, 0, 0);
	
    
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    
    fs::path clPath = App::getResourcePath("WaveParticle.cl");
	mOpenCl.loadProgramFromFile(clPath.string());
	mKernelUpdate = mOpenCl.loadKernel("updateParticle");
	
	
	//mClBufParticles.initBuffer(sizeof(clParticle) * kMagnetoNumParticles, CL_MEM_READ_WRITE, mParticles);
    mClBufParticles.initBuffer(sizeof(clParticle) * kMagnetoNumParticles, CL_MEM_READ_WRITE, mParticles);
    mClBufNodes.initBuffer(sizeof(clNode) * kMagnetoMaxNodes, CL_MEM_READ_ONLY, NULL);
    
    mClBufPosBuffer.initFromGLObject(mVbo[0]);
	mClBufColorBuffer.initFromGLObject(mVbo[1]);
	
	mKernelUpdate->setArg(ARG_PARTICLES, mClBufParticles.getCLMem());
    mKernelUpdate->setArg(ARG_POS_BUFFER, mClBufPosBuffer.getCLMem());
    mKernelUpdate->setArg(ARG_COLOR_BUFFER, mClBufColorBuffer.getCLMem());
}

void Magnetosphere::reset()
{
    updateParams();
    updateNodes();
}

void Magnetosphere::resize()
{
    if(mUseFbo) 
    {
		mRenderDimensions.x = mFboNew.getWidth();
		mRenderDimensions.y = mFboNew.getHeight();
	} 
    else 
    {
		mRenderDimensions.x = mApp->getViewportWidth();
		mRenderDimensions.y = mApp->getViewportHeight();
	}
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
    params.addParam("Blending", &mEnableBlending, "");
    params.addParam("Additive Blending", &mAdditiveBlending, "");

    params.addParam("Node Radius", &mNodeRadius, "");
    params.addParam("Draw Nodes", &mDoDrawNodes, "");
    
    params.addParam("Point Size", &mPointSize, "");
    params.addParam("Draw Points", &mDoDrawPoints, "");
    params.addParam("Point Sprites", &mUseImageForPoints, "");
    
    params.addParam("DrawLines", &mDoDrawLines, "");
    params.addParam("Line Width", &mLineWidth, "");
    //params.addParam("K Particles", &mNumParticles, "");
    
    params.addParam("Time Speed", &mTimeSpeed, "step=0.01 min=0.0 max=1.0");
    params.addParam("Die Speed", &mDieSpeed, "step=0.01 min=0.0 max=1.0");
    params.addParam("Mass Min", &mMassMin, "step=0.01 min=0.0 max=1.0");
    params.addParam("Momentum", &mMomentum, "step=0.01 min=0.0 max=1.0");
    params.addParam("SpreadMin", &mSpreadMin, "step=0.01 min=0.0 max=1.0");
    params.addParam("SpreadMax", &mSpreadMax, "step=0.01 min=0.0 max=1.0");
    params.addParam("NodeAttractMin", &mNodeAttractMin, "step=0.01 min=0.0 max=1.0");
    params.addParam("NodeAttractMax", &mNodeAttractMax, "step=0.01 min=0.0 max=1.0");
    params.addParam("WaveAmpMin", &mWaveAmpMin, "step=0.01 min=0.0 max=1.0");
    params.addParam("WaveAmpMax", &mWaveAmpMax, "step=0.01 min=0.0 max=1.0");
    params.addParam("WaveFreqMin", &mWaveFreqMin, "step=0.01 min=0.0 max=1.0");
    params.addParam("WaveFreqMax", &mWaveFreqMax, "step=0.01 min=0.0 max=1.0");
    params.addParam("WavePosMult", &mWavePosMult, "step=0.01 min=0.0 max=10.0");
    params.addParam("WaveVelMult", &mWaveVelMult, "step=0.01 min=0.0 max=10.0");
    
    params.addParam("Color Taper", &mColorTaper, "step=0.01 min=0.0 max=1.0");
    //TODO:color
    
    params.addParam("NodeBrightness", &mNodeBrightness, "step=0.01 min=0.0 max=1.0");
    
    // trail
    params.addParam("mUseFbo", &mUseFbo, "");
    params.addParam("DoBlur", &mDoBlur, "");
    
    params.addParam("mFboScaleDown", &mFboScaleDown, "step=0.01 min=1.0 max=8.0");
    params.addParam("mBlurAmount", &mBlurAmount, "min=1 max=50");
    params.addParam("mTrailAudioDistortMin", &mTrailAudioDistortMin, "step=0.01 min=0.0 max=1.0");
    params.addParam("mTrailAudioDistortMax", &mTrailAudioDistortMax, "step=0.01 min=0.0 max=10.0");
    params.addParam("mTrailWaveFreqMin", &mTrailWaveFreqMin, "step=0.01 min=0.0 max=1.0");
    params.addParam("mTrailWaveFreqMax", &mTrailWaveFreqMax, "step=0.01 min=0.0 max=1.0");
    params.addParam("mTrailDistanceAffectsWave", &mTrailDistanceAffectsWave, "");
    params.addParam("mFadeSpeed", &mFadeSpeed, "step=0.01 min=0.0 max=1.0");
    
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
	//mDimensions.x = mApp->getViewportWidth();
	//mDimensions.y = mApp->getViewportHeight();
    
    updateAudioResponse();
    
    mAnimTime += mTimeSpeed;
    mKernelUpdate->setArg(ARG_TIME, mAnimTime);
    
    //		updateParams();
    
    //		if(nodesNeedUpdating) 
    updateNodes();
    
    mKernelUpdate->run1D(mNumParticles);
	
	//mKernelUpdate->setArg(2, mMousePos);
	//mKernelUpdate->setArg(3, mDimensions);
	//mKernelUpdate->run1D(kMagnetoNumParticles);
    
    //updateHud();
    
    // debug info
    char buf[256];
    snprintf(buf, 256, "particles: %d", mNumParticles);
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
    
    mNodesNeedUpdating = true;
}

void Magnetosphere::handleMouseUp( const MouseEvent& event )
{
	mIsMousePressed = false;
    
    mNodesNeedUpdating = true;
}

void Magnetosphere::handleMouseDrag( const MouseEvent& event )
{
	mMousePos.x = event.getPos().x;
    mMousePos.y = event.getPos().y;
    
    mNodesNeedUpdating = true;
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
	glDrawArrays(GL_POINTS, 0, kMagnetoNumParticles);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    */
    
    if(!mUseFbo)
        drawParticles();
    
    if(mUseFbo) 
    {
		drawOffscreen();
        //		
        //		
        //		if(blurAlpha) {
        //			glColor3f(blurAlpha, blurAlpha, blurAlpha);
        //			glEnable(GL_BLEND);
        //			glBlendFunc(GL_ONE, GL_ONE);
        //			fboBlur.draw(0, 0, ofGetWidth(), ofGetHeight());
        //		}
        //		
        //		if(origAlpha) {
        //			glColor3f(origAlpha, origAlpha, origAlpha);
        //			glEnable(GL_ALPHA);
        //			glBlendFunc(GL_ONE, GL_ONE);
        //			fboNew.draw(0, 0, ofGetWidth(), ofGetHeight());
        //		}
		glColor3f( 1, 1, 1 );
        gl::enableAdditiveBlending();
        gl::setMatricesWindow( mApp->getWindowSize() );
        gl::draw( mFboComp.getTexture(), getWindowBounds() );
	}
    else
    {
        drawStrokes();
    }
	
    glPopMatrix();
}

//
//
//

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
	mKernelUpdate->setArg(ARG_NODES, mClBufNodes.getCLMem());
	mKernelUpdate->setArg(ARG_NUM_NODES, mNumNodes);
	
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

void Magnetosphere::updateParams() 
{
	resize();
	
	mKernelUpdate->setArg(ARG_COLOR, mColor);
	mKernelUpdate->setArg(ARG_COLOR_TAPER, mColorTaper);
	mKernelUpdate->setArg(ARG_MOMENTUM, mMomentum);
	mKernelUpdate->setArg(ARG_DIESPEED, mDieSpeed);
	mKernelUpdate->setArg(ARG_WAVE_POS_MULT, mWavePosMult);
	mKernelUpdate->setArg(ARG_WAVE_VEL_MULT, mWaveVelMult);
	mKernelUpdate->setArg(ARG_MASS_MIN, mMassMin);
	
	
	//mNumParticles = 1 << mNumParticlesPower;
    mNumParticles = kMagnetoNumParticles;
	if(mNumParticles>kMagnetoNumParticles) mNumParticles = kMagnetoNumParticles;
}	


void Magnetosphere::preRender() 
{
    
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
}






void Magnetosphere::drawParticles() 
{
    preRender();
    
    if(mDoDrawNodes) 
    {
        drawNodes();
    }
    
    //		openCL.finish();
    mOpenCl.flush();
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[0]);
    glVertexPointer(2, GL_FLOAT, 0, 0);
    
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, mVbo[1]);
    glColorPointer(4, GL_FLOAT, 0, 0);
    
    if(mDoDrawLines) 
    {
        glDrawElements(GL_LINES, mNumParticles * kMagnetoMaxTrailLength, GL_UNSIGNED_INT, mIndices);
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
}

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
     */
    
    gl::setViewport( viewport );
}
