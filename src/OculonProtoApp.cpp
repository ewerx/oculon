/*
 *  OculonProtoApp.h
 *  OculonProtoApp
 *
 *  Created by Ehsan on 11-10-16.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "cinder/gl/gl.h"
#include "cinder/audio/Input.h"
#include "cinder/Camera.h"
#include "cinder/params/Params.h"
#include <iostream>
#include <vector>
#include "OculonProtoApp.h"
#include "AudioInput.h"
#include "InfoPanel.h"
#include "Orbiter.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void OculonProtoApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 800, 600 );
	settings->setFrameRate( 60.0f );
	settings->setFullScreen( false );
}

void OculonProtoApp::setup()
{
    // render
    gl::enableDepthWrite();
	gl::enableDepthRead();
	gl::enableAlphaBlending();
    glDisable( GL_TEXTURE_2D );
    
    // setup our default camera, looking down the z-axis
	//mCam.lookAt( Vec3f( 0.0f, 0.0f, 100.0f ), Vec3f::zero() );
    
    // load assets
    //gl::Texture earthDiffuse	= gl::Texture( loadImage( loadResource( RES_EARTHDIFFUSE ) ) );
	//gl::Texture earthNormal		= gl::Texture( loadImage( loadResource( RES_EARTHNORMAL ) ) );
	//gl::Texture earthMask		= gl::Texture( loadImage( loadResource( RES_EARTHMASK ) ) );
	//earthDiffuse.setWrap( GL_REPEAT, GL_REPEAT );
	//earthNormal.setWrap( GL_REPEAT, GL_REPEAT );
	//earthMask.setWrap( GL_REPEAT, GL_REPEAT );
    //mSphereShader = gl::GlslProg( loadResource( RES_PASSTHRU_VERT ), loadResource( RES_EARTH_FRAG ) );
    
    // audio input
    mAudioInput.Init(this);
    
    // debug
    mRenderInfoPanel = false;
	//glDisable( GL_TEXTURE_2D );
    
    mParams = params::InterfaceGl( "Parameters", Vec2i( 200, 400 ) );
    //mParams.addParam( "Cube Color", &mColor, "" );
    //mParams.addSeparator();	
	//mParams.addParam( "Light Direction", &mLightDirection, "" );
    
    //mParams.addText( "text", "label=`Temp`" );
    //mParams.addParam( "Cube Size", &mObjSize, "min=0.1 max=20.5 step=0.5 keyIncr=z keyDecr=Z" );
    //mParams.addParam( "Gravity Constant", &Orbiter::sGravityConstant, "min=1.0 max=9999999999.0 step=1.0 keyIncr=g keyDecr=G" );
    //mParams.addParam( "Mass Ratio", &mOrbiter.mMassRatio, "min=1.0 max=9999999999.0 step=1.0 keyIncr=m keyDecr=M" );
    //mParams.addParam( "Escape Velocity", &mOrbiter.mEscapeVelocity, "min=-99999999999.0 max=9999999999.0 step=0.1 keyIncr=e keyDecr=E" );
    //mParams.hide();
    
    mLastElapsedSeconds = getElapsedSeconds();
    
    setupScenes();
}

void OculonProtoApp::setupScenes()
{
    mScenes.clear();
    
    //TODO: serialization
    Scene* orbiter = new Orbiter();
    orbiter->init(this);
    mScenes.push_back( orbiter );
    
    for (vector<Scene*>::iterator sceneIt = mScenes.begin(); 
         sceneIt != mScenes.end();
         ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        assert( scene != NULL );
        if( scene )
        {
            scene->setup();
            scene->setupParams(mParams);
        }
    }
}

void OculonProtoApp::resize( ResizeEvent event )
{
    //mCam.setAspectRatio( getWindowAspectRatio() );
    mCam.lookAt( Vec3f( 0.0f, 0.0f, 750.0f ), Vec3f::zero(), Vec3f(0.0f, 1.0f, 0.0f) );
	mCam.setPerspective( 60, getWindowAspectRatio(), 1, 1000 );
	gl::setMatrices( mCam );
}

void OculonProtoApp::mouseMove( MouseEvent event )
{
	mMousePos.x = event.getX() - getWindowWidth() * 0.5f;
	mMousePos.y = getWindowHeight() * 0.5f - event.getY();
}

void OculonProtoApp::mouseDown( MouseEvent event )
{
}

void OculonProtoApp::keyDown( KeyEvent event )
{
    switch( event.getChar() )
    {
        case ' ':
            mScenes[0]->reset();//TODO: unsafe
            break;
            
        // toggle pause-all
        case 'p':
        case 'P':
        {
            for (vector<Scene*>::iterator sceneIt = mScenes.begin(); 
                 sceneIt != mScenes.end();
                 ++sceneIt )
            {
                Scene* scene = (*sceneIt);
                assert( scene != NULL );
                if( scene )
                {
                    scene->setActive( !scene->isActive() );
                }
            }
            break;
        }

        case '/':
        case '?':
            mInfoPanel.toggleState();
            break;
        default:
            break;
    }
}

void OculonProtoApp::update()
{
    char buf[64];
    snprintf(buf, 64, "%.2ffps", getAverageFps());
    mInfoPanel.addLine( buf, Color(0.5f, 0.5f, 0.5f) );
    
    double dt = getElapsedSeconds() - mLastElapsedSeconds;
    mLastElapsedSeconds = getElapsedSeconds();
    
    mAudioInput.update();
    
    for (vector<Scene*>::iterator sceneIt = mScenes.begin(); 
         sceneIt != mScenes.end();
         ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        if( scene && scene->isActive() )
        {
            scene->update(dt);
        }
    }
    
    mInfoPanel.update();
}

void OculonProtoApp::draw()
{
    // 3D scene
    glPushMatrix();
    {
        
        
        //glLoadIdentity();
        //glEnable( GL_TEXTURE_2D );
        //glEnable( GL_LIGHTING );
        //glEnable( GL_LIGHT0 );	
        //GLfloat lightPosition[] = { -mLightDirection.x, -mLightDirection.y, -mLightDirection.z, 0.0f };
        //glLightfv( GL_LIGHT0, GL_POSITION, lightPosition );
        //glMaterialfv( GL_FRONT, GL_DIFFUSE,	mColor );
        
        /*
        ColorA color( 0.25f, 0.5f, 1.0f, 1.0f );
        gl::setMatrices( mCam );
        //gl::rotate( mObjOrientation );
        gl::color( mColor );
        gl::drawCube( Vec3f::zero(), Vec3f( mObjSize, mObjSize, mObjSize ) );
        */

        for (vector<Scene*>::iterator sceneIt = mScenes.begin(); 
             sceneIt != mScenes.end();
             ++sceneIt )
        {
            Scene* scene = (*sceneIt);
            assert( scene != NULL );
            if( scene && scene->isVisible() )
            {
                scene->draw();
            }
        }
    }
    glPopMatrix();
    
    // 2D visuals
    glPushMatrix();
    static bool drawGraphs = false;
    if( drawGraphs )
    {
        gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
        // clear out the window with black
        //gl::clear( Color( 0, 0, 0 ) ); 
    
        drawWaveform( mAudioInput.GetPcmBuffer() );
        glTranslatef( 0.0f, 200.0f, 0.0f );
        drawFft( mAudioInput.GetFftDataRef() );
    }
    glPopMatrix();
     
    
    // debug
    drawInfoPanel();
	params::InterfaceGl::draw();
}

// new methods
void OculonProtoApp::drawWaveform( audio::PcmBuffer32fRef pcmBufferRef )
{
    if( ! pcmBufferRef ) 
    {
		return;
	}
	
	uint32_t bufferSamples = pcmBufferRef->getSampleCount();
	audio::Buffer32fRef leftBuffer = pcmBufferRef->getChannelData( audio::CHANNEL_FRONT_LEFT );
	audio::Buffer32fRef rightBuffer = pcmBufferRef->getChannelData( audio::CHANNEL_FRONT_RIGHT );
    
	int displaySize = getWindowWidth();
	int endIdx = bufferSamples;
	
	//only draw the last 1024 samples or less
	int32_t startIdx = ( endIdx - 1024 );
	startIdx = math<int32_t>::clamp( startIdx, 0, endIdx );
	
	float scale = displaySize / (float)( endIdx - startIdx );
	
	PolyLine<Vec2f>	spectrum_right;
    PolyLine<Vec2f> spectrum_left;
	
	for( uint32_t i = startIdx, c = 0; i < endIdx; i++, c++ ) 
    {
		float y = ( ( rightBuffer->mData[i] - 1 ) * - 100 );
		spectrum_right.push_back( Vec2f( ( c * scale ), y ) );
        y = ( ( leftBuffer->mData[i] - 1 ) * - 100 );
		spectrum_left.push_back( Vec2f( ( c * scale ), y ) );
	}
    gl::color( Color( 1.0f, 0.5f, 0.25f ) );
	gl::draw( spectrum_right );
    gl::color( Color( 0.5f, 0.5f, 1.0f ) );
    gl::draw( spectrum_left );
}

void OculonProtoApp::drawFft( std::shared_ptr<float> fftDataRef )
{
    uint16_t bandCount = 512;
	float ht = 1000.0f;
	float bottom = 150.0f;
	
	if( ! fftDataRef ) 
    {
		return;
	}
	
	float * fftBuffer = fftDataRef.get();
	
	for( int i = 0; i < ( bandCount ); i++ ) 
    {
		float barY = fftBuffer[i] / bandCount * ht;
		glBegin( GL_QUADS );
        glColor3f( 255.0f, 255.0f, 0.0f );
        glVertex2f( i * 3, bottom );
        glVertex2f( i * 3 + 1, bottom );
        glColor3f( 0.0f, 255.0f, 0.0f );
        glVertex2f( i * 3 + 1, bottom - barY );
        glVertex2f( i * 3, bottom - barY );
		glEnd();
	}
}

void OculonProtoApp::drawInfoPanel()
{
	glDisable( GL_LIGHTING );
	glEnable( GL_TEXTURE_2D );
	glColor4f( 1, 1, 1, 1 );
	
	gl::pushMatrices();
        gl::setMatricesWindow( getWindowSize() );
        mInfoPanel.render( Vec2f( getWindowWidth(), getWindowHeight() ) );
	gl::popMatrices();
    
	glDisable( GL_TEXTURE_2D );
}

CINDER_APP_BASIC( OculonProtoApp, RendererGl )
