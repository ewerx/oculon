/*
 *  OculonApp.h
 *  OculonApp
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
#include "OculonApp.h"
#include "AudioInput.h"
#include "InfoPanel.h"
#include "Orbiter.h"
#include "AudioTest.h"
#include "MindWaveTest.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void OculonApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 800, 600 );
	settings->setFrameRate( 60.0f );
	settings->setFullScreen( false );
}

void OculonApp::setup()
{
    mIsPresentationMode = false;
    
    // render
    gl::enableDepthWrite();
	gl::enableDepthRead();
	gl::enableAlphaBlending();
    glDisable( GL_TEXTURE_2D );
    
    // setup our default camera, looking down the z-axis
	//mCam.lookAt( Vec3f( 0.0f, 0.0f, 750.0f ), Vec3f::zero(), Vec3f(0.0f, 1.0f, 0.0f) );
    CameraPersp cam;
    cam.setEyePoint( Vec3f(0.0f, 0.0f, 750.0f) );
	cam.setCenterOfInterestPoint( Vec3f::zero() );
	cam.setPerspective( 60.0f, getWindowAspectRatio(), 1.0f, 20000.0f );
    mMayaCam.setCurrentCam( cam );
    
    // load assets
    //gl::Texture earthDiffuse	= gl::Texture( loadImage( loadResource( RES_EARTHDIFFUSE ) ) );
	//gl::Texture earthNormal		= gl::Texture( loadImage( loadResource( RES_EARTHNORMAL ) ) );
	//gl::Texture earthMask		= gl::Texture( loadImage( loadResource( RES_EARTHMASK ) ) );
	//earthDiffuse.setWrap( GL_REPEAT, GL_REPEAT );
	//earthNormal.setWrap( GL_REPEAT, GL_REPEAT );
	//earthMask.setWrap( GL_REPEAT, GL_REPEAT );
    //mSphereShader = gl::GlslProg( loadResource( RES_PASSTHRU_VERT ), loadResource( RES_EARTH_FRAG ) );
    
    // audio input
    mAudioInput.setup();
    
    mMindWave.setup();
    
    // debug
	//glDisable( GL_TEXTURE_2D );
    
    mParams = params::InterfaceGl( "Parameters", Vec2i( 300, 100 ) );
    //mParams.setOptions("","position='200 10'");
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

void OculonApp::setupScenes()
{
    mScenes.clear();
    
    //TODO: serialization
    // Orbiter
    Scene* scene = new Orbiter();
    scene->init(this);
    mScenes.push_back( scene );
    
    // AudioTest
    scene = new AudioTest();
    scene->init(this);
    scene->toggleActiveVisible(); // start disabled (should be default?)
    mScenes.push_back( scene );
    
    // MindWaveTet
    scene = new MindWaveTest();
    scene->init(this);
    mScenes.push_back(scene);
    
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

void OculonApp::resize( ResizeEvent event )
{
    CameraPersp cam = mMayaCam.getCamera();
    cam.setAspectRatio( getWindowAspectRatio() );
    mMayaCam.setCurrentCam( cam );
    //mCam.lookAt( Vec3f( 0.0f, 0.0f, 750.0f ), Vec3f::zero(), Vec3f(0.0f, 1.0f, 0.0f) );
	//mCam.setPerspective( 60, getWindowAspectRatio(), 1, 1000 );
	//gl::setMatrices( mCam );
}

void OculonApp::mouseMove( MouseEvent event )
{
	mMousePos.x = event.getX() - getWindowWidth() * 0.5f;
	mMousePos.y = getWindowHeight() * 0.5f - event.getY();
}

void OculonApp::mouseDown( MouseEvent event )
{
    // let the camera handle the interaction
	mMayaCam.mouseDown( event.getPos() );
}

void OculonApp::mouseDrag( MouseEvent event )
{
    // let the camera handle the interaction
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void OculonApp::keyDown( KeyEvent event )
{
    switch( event.getChar() )
    {
        case ' ':
            mScenes[0]->reset();//TODO: active scene?
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
            
        case 'l':
        case 'L':
            setPresentationMode( !mIsPresentationMode );
            break;

        case '/':
        case '?':
            mInfoPanel.toggleState();
            break;
            
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        {
            char index = event.getChar() - '1';
            
            if( index < mScenes.size() && mScenes[index] != NULL )
            {
                mScenes[index]->toggleActiveVisible();
            }
        }
        default:
            break;
    }
}

void OculonApp::update()
{
    char buf[64];
    snprintf(buf, 64, "%.2ffps", getAverageFps());
    mInfoPanel.addLine( buf, Color(0.5f, 0.5f, 0.5f) );
    
    double dt = getElapsedSeconds() - mLastElapsedSeconds;
    mLastElapsedSeconds = getElapsedSeconds();
    
    mAudioInput.update();
    mMidiInput.update();
    mMindWave.update();
    
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

void OculonApp::draw()
{
    gl::clear( Colorf(0.0f, 0.0f, 0.0f) );
    
    // 3D scenes
    glPushMatrix();
    {
        // setup camera
        gl::setMatrices( mMayaCam.getCamera() );
        // enable depth buffer
        // enable the depth buffer (after all, we are doing 3D)
        gl::enableDepthRead();
        gl::enableDepthWrite();

        // render scenes
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
    
    // debug
    if( !mIsPresentationMode )
    {
        drawInfoPanel();
        params::InterfaceGl::draw();
    }
}

void OculonApp::drawInfoPanel()
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

void OculonApp::setPresentationMode( bool enabled )
{
    setFullScreen(enabled);
    mInfoPanel.setVisible(!enabled);
    mIsPresentationMode = enabled;
}

CINDER_APP_BASIC( OculonApp, RendererGl )
