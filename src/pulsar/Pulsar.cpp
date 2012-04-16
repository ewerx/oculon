/*
 *  Pulsar.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-11-26.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Pulsar.h"

#include "Resources.h"
#include "AudioInput.h" // compile errors if this is not before App.h
#include "OculonApp.h"//TODO: fix this dependency
#include "cinder/Rand.h"


using namespace ci;

Pulsar::Pulsar()
: Scene("Pulsar")
{
}

Pulsar::~Pulsar()
{
}

void Pulsar::setup()
{
    mParticleStream.setup(this);
    
    // textures
    mEmitterTexture		= gl::Texture( loadImage( loadResource( RES_NORMAL ) ) );
	//mNormalTexture		= gl::Texture( loadImage( loadResource( RES_NORMAL ) ) );
	//mHeightTexture		= gl::Texture( loadImage( loadResource( RES_BUMP ) ) );
	//mSpecTexture		= gl::Texture( loadImage( loadResource( RES_SPEC_EXPONENT ) ) );
    
    mEmitterTexture.setWrap( GL_REPEAT, GL_REPEAT );
	//mNormalTexture.setWrap( GL_REPEAT, GL_REPEAT );
	//mHeightTexture.setWrap( GL_REPEAT, GL_REPEAT );
	//mSpecTexture.setWrap( GL_REPEAT, GL_REPEAT );
    
    /*
    try 
    {
		mPulsarShader = gl::GlslProg( loadResource( RES_EMITTER_VERT ), loadResource( RES_EMITTER_FRAG ) );
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
    */
    
    // physics
    mRadius = 50.0f;
    mRotationVelocity = 400.0f;
    mHeat = 0.49f;
    
    reset();
}

void Pulsar::reset()
{
}

void Pulsar::resize()
{
}

//void Pulsar::setupMidiMapping()
//{
    // setup MIDI inputs for learning
    //mMidiMap.registerMidiEvent("orb_gravity", MidiEvent::TYPE_VALUE_CHANGE, this, &Orbiter::handleGravityChange);
    //mMidiMap.registerMidiEvent("orb_timescale", MidiEvent::TYPE_VALUE_CHANGE, this, &Orbiter::handleGravityChange);
    //mMidiMap.beginLearning();
    // ... or load a MIDI mapping
    //mMidiInput.setMidiKey("gravity", channel, note);
//}

void Pulsar::setupDebugInterface()
{
    mDebugParams.addText( "pulsar", "label=`Pulsar`" );
    mDebugParams.addParam("Rotation Speed", &mRotationVelocity, "step=10 keyIncr=t keyDecr=y");
    //mDebugParams.addParam("Follow Target", &mFollowTargetIndex, "keyIncr=] keyDecr=[");
    //mDebugParams.addParam("Time Scale", &mTimeScale, "step=86400.0 KeyIncr=. keyDecr=,");
    //mDebugParams.addParam("Max Radius Mult", &Orbiter::sMaxRadiusMultiplier, "step=0.1");
    //mDebugParams.addParam("Frames to Avg", &Orbiter::sNumFramesToAvgFft, "step=1");
    //mDebugParams.addParam("Trails - Smooth", &Orbiter::sUseSmoothLines, "key=s");
    //mDebugParams.addParam("Trails - Ribbon", &Orbiter::sUseTriStripLine, "key=t");
    //mDebugParams.addParam("Trails - LengthFact", &Orbiter::sMinTrailLength, "keyIncr=l keyDecr=;");
    //mDebugParams.addParam("Trails - Width", &Orbiter::sTrailWidth, "keyIncr=w keyDecr=q step=0.1");
    //mDebugParams.addParam("Planet Grayscale", &Orbiter::sPlanetGrayScale, "keyIncr=x keyDecr=z step=0.05");
    //mDebugParams.addParam("Real Sun Radius", &Orbiter::sDrawRealSun, "key=r");
    //mDebugParams.addSeparator();
    //mDebugParams.addParam("Frustum Culling", &mEnableFrustumCulling, "keyIncr=f");
}

void Pulsar::setupInterface()
{
    
}

void Pulsar::update(double dt)
{
    mParticleStream.update(dt);
    
    updateAudioResponse();
    //updateHud();
    
    mRotationAngle += mRotationVelocity*dt;
    mRotation.set( Vec3f(1.0f, 1.0f, 0.0f), mRotationAngle );
    
    generateParticles();
    
    // debug info   
    char buf[256];
    snprintf(buf, 256, "particles: %d", mParticleStream.getParticleCount());
    mApp->getInfoPanel().addLine(buf, Color(0.75f, 0.5f, 0.5f));    
    
    Scene::update(dt);
}

//
// handleKeyDown
//
bool Pulsar::handleKeyDown(const KeyEvent& keyEvent)
{
    bool handled = true;
    
    switch (keyEvent.getChar()) 
    {/*
        case ' ':
            reset();
            break;
        case 'g':
            generateParticles();
            break;
        case 'q':
            mParticleStream.toggleParticleDrawMode();
            break;*/
        default:
            handled = false;
            break;
    }
    
    return handled;
}

//
//
//
void Pulsar::updateAudioResponse()
{
    AudioInput& audioInput = mApp->getAudioInput();
    std::shared_ptr<float> fftDataRef = audioInput.getFftDataRef();
    
    //unsigned int bandCount = audioInput.getFftBandCount();
    //float* fftBuffer = fftDataRef.get();
    
    //int bodyIndex = 0;
    
    //TODO
}

void Pulsar::draw()
{
    glPushMatrix();
    //gl::enableDepthWrite( true );
	//gl::enableDepthRead( true );
	//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glRotatef((float)mRotationAngle, 1.0f, 0.0f, 0.0f);
    drawStar();

	//glRotatef(mRotationAngle, 0.0f, 1.0f, 0.0f);

    mParticleStream.draw();
    glPopMatrix();
}

void Pulsar::drawStar()
{
    glPushMatrix();
    /*
    glEnable( GL_TEXTURE_2D );
	
    Vec3f lightDir( 0.0f, 0.0f, 1.0f );
	
	mEmitterTexture.bind( 0 );
	mNormalTexture.bind( 1 );
	mHeightTexture.bind( 2 );
	mSpecTexture.bind( 3 );
    
	mPulsarShader.bind();
	mPulsarShader.uniform( "texDiffuse", 0 );
	mPulsarShader.uniform( "texNormal", 1 );
	mPulsarShader.uniform( "texHeight", 2 );
	mPulsarShader.uniform( "texSpec", 3 );
    mPulsarShader.uniform( "isPressed", 1 );
    
    //TODO: figure out what these do
    mPulsarShader.uniform( "heat", mHeat );
	mPulsarShader.uniform( "mouseVel", 0.00025f );
	mPulsarShader.uniform( "spinSpeed", (float)mRotationVelocity/200.f );
	mPulsarShader.uniform( "counter", (float)mFrameCount );
	mPulsarShader.uniform( "lightDir", lightDir );
	mPulsarShader.uniform( "minHeight", 0 );
    
    glRotated(mRotationAngle, 0.0f, 1.0f, 0.0f);
    glColor4f(1.0f,1.0f,1.0f,0.0f);
    gl::drawSphere(Vec3f::zero(), (float)mRadius, 32);
    
    gl::enableDepthWrite( false );
	glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
    glColor4f(1.0f,1.0f,1.0f,0.1f);
	for( int i=0; i<20; i++ )
    {
		mPulsarShader.uniform( "minHeight", (float)i/20.0f );
        
        gl::drawSphere(Vec3f::zero(), (float)mRadius + 2.0f + i*0.1f, 32);
		//mEmitter.render( mIsMouseDown, 52 + i * 0.1f, 0.1f );
	}
	
	mPulsarShader.unbind();
    
    glDisable( GL_TEXTURE_2D );
    */
    glDisable(GL_LIGHTING);
    mEmitterTexture.enableAndBind();
    //glRotatef(mRotationAngle, 0.0f, 1.0f, 0.0f);
    //glRotatef((float)mRotationAngle, 1.0f, 0.0f, 0.0f);
    glColor4f(1.0f,1.0f,1.0f,1.0f);
    gl::drawSphere(Vec3f::zero(), (float)mRadius, 32);
    mEmitterTexture.disable();
    glPopMatrix();
    ++mFrameCount;
}

void Pulsar::generateParticles()
{
    Vec3f pos = Vec3f(0.0f, 0.0f, 0.0f);
    //pos.rotateX(mRotationAngle);
    Vec3f vel = Vec3f(0.0f, 50.f, 0.0f);
    //vel.rotateX(mRotationAngle);
    float particleRadius = 10.0f;//mRadius;
    int depth = pos.y - 380;
    float per = depth/340.0f;
    //Vec3f vel = mEmitter.mVel * per;
    //vel.y *= 0.02f;
    int numParticlesToSpawn = Rand::randInt(40,100);
    if( Rand::randFloat() < 0.02f )
    {
        //numParticlesToSpawn *= 5;
    }
    mParticleStream.addParticles( numParticlesToSpawn, pos, vel * per, particleRadius );
    mParticleStream.addParticles( numParticlesToSpawn, -pos, -vel * per, particleRadius );
}
