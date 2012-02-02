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
#include "cinder/Filesystem.h"
#include "cinder/Utilities.h"
#include <iostream>
#include <vector>
#include <boost/format.hpp>
#include "OculonApp.h"
#include "AudioInput.h"
#include "InfoPanel.h"

// scenes
#include "Orbiter.h"
#include "Magnetosphere.h"
#include "Pulsar.h"
#include "Binned.h"
// test scenes
#include "AudioTest.h"
#include "MindWaveTest.h"
#include "MovieTest.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace boost;

void OculonApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1024, 768 );
	settings->setFrameRate( 60.0f );
	settings->setFullScreen( false );
    settings->enableSecondaryDisplayBlanking(false);
}

void OculonApp::setup()
{
    console() << "[main] initializing...\n";
    
    mIsPresentationMode = false;
    mUseMayaCam = true;
    mEnableMindWave = false;
    mIsCapturingVideo = false;
    mEnableOscServer = true;
    
    // render
    gl::enableDepthWrite();
	gl::enableDepthRead();
	gl::enableAlphaBlending();
    glDisable( GL_TEXTURE_2D );
    //wireframe
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    
    // setup our default camera, looking down the z-axis
	//mCam.lookAt( Vec3f( 0.0f, 0.0f, 750.0f ), Vec3f::zero(), Vec3f(0.0f, 1.0f, 0.0f) );
    mCam.setEyePoint( Vec3f(0.0f, 0.0f, 750.0f) );
	mCam.setCenterOfInterestPoint( Vec3f::zero() );
	mCam.setPerspective( 75.0f, getWindowAspectRatio(), 1.0f, 200000.0f );
    mMayaCam.setCurrentCam( mCam );
    
    // params
    mParams = params::InterfaceGl( "Parameters", Vec2i( 350, 150 ) );
    //mParams.setOptions("","position='10 600'");
    //mParams.addParam( "Cube Color", &mColor, "" );
    //mParams.addSeparator();	
	//mParams.addParam( "Light Direction", &mLightDirection, "" );
    
    //mParams.addText( "text", "label=`Temp`" );
    //mParams.addParam( "Cube Size", &mObjSize, "min=0.1 max=20.5 step=0.5 keyIncr=z keyDecr=Z" );
    //mParams.addParam( "Gravity Constant", &Orbiter::sGravityConstant, "min=1.0 max=9999999999.0 step=1.0 keyIncr=g keyDecr=G" );
    //mParams.addParam( "Mass Ratio", &mOrbiter.mMassRatio, "min=1.0 max=9999999999.0 step=1.0 keyIncr=m keyDecr=M" );
    //mParams.addParam( "Escape Velocity", &mOrbiter.mEscapeVelocity, "min=-99999999999.0 max=9999999999.0 step=0.1 keyIncr=e keyDecr=E" );
    //mParams.hide();
    
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
    
    if( mEnableMindWave )
    {
        mMindWave.setup();
    }
    mMidiInput.setEnabled(false);
    
    if( mEnableOscServer )
    {
        mOscServer.setup();
    }
    
    // debug
	//glDisable( GL_TEXTURE_2D );
    
    mLastElapsedSeconds = getElapsedSeconds();
    mElapsedSecondsThisFrame = 0.0f;
    
    setupScenes();
}

void OculonApp::shutdown()
{
    console() << "[main] shutting down...\n";
    
    for (SceneList::iterator sceneIt = mScenes.begin(); 
         sceneIt != mScenes.end();
         ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        if( scene )
        {
            scene->shutdown();
            delete scene;
        }
    }
    
    mScenes.clear();
    
    if( mEnableOscServer )
    {
        mOscServer.shutdown();
    }
    
    mAudioInput.shutdown();
}

void OculonApp::addScene(Scene* scene, bool startActive)
{
    scene->init(this);
    mScenes.push_back(scene);
    if( startActive )
    {
        scene->toggleActiveVisible();
    }
}

void OculonApp::setupScenes()
{
    console() << "[main] creating scenes...\n";
    
    mScenes.clear(); 
    
    int sceneId = 0;
    
    //TODO: serialization    
    
    // Orbiter
    console() << ++sceneId << ": Orbiter\n";
    addScene( new Orbiter() );
    
    // Pulsar
    console() << ++sceneId << ": Pulsar\n";
    addScene( new Pulsar() );
    
    // Magnetosphere
    console() << ++sceneId << ": Magneto\n";
    addScene( new Magnetosphere() );
    
    // Binned
    console() << ++sceneId << ": Binned\n";
    addScene( new Binned(), true );
    
    // AudioTest
    console() << ++sceneId << ": AudioTest\n";
    addScene( new AudioTest() );
    
    // MovieTest
    console() << ++sceneId << ": MovieTest\n";
    addScene( new MovieTest() );

    
    if( mEnableMindWave )
    {
        console() << "\t" << sceneId << ": MindWave\n";
        addScene( new MindWaveTest() );
    }
}

void OculonApp::resize( ResizeEvent event )
{
    if( mUseMayaCam )
    {
        mCam = mMayaCam.getCamera();
    }
    mCam.setAspectRatio( getWindowAspectRatio() );
    mMayaCam.setCurrentCam( mCam );
    
    for (SceneList::iterator sceneIt = mScenes.begin(); 
        sceneIt != mScenes.end();
        ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        if( scene && scene->isActive() )
        {
            scene->resize();
        }
    }
}

void OculonApp::mouseMove( MouseEvent event )
{
	mMousePos.x = event.getX() - getWindowWidth() * 0.5f;
	mMousePos.y = getWindowHeight() * 0.5f - event.getY();
}

void OculonApp::mouseDown( MouseEvent event )
{
    if( mUseMayaCam )
    {
        // let the camera handle the interaction
        mMayaCam.mouseDown( event.getPos() );
    }
    
    for (SceneList::iterator sceneIt = mScenes.begin(); 
         sceneIt != mScenes.end();
         ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        if( scene && scene->isActive() )
        {
            scene->handleMouseDown(event);
        }
    }
}

void OculonApp::mouseDrag( MouseEvent event )
{
    if( mUseMayaCam )
    {
        // let the camera handle the interaction
        mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
    }
    
    for (SceneList::iterator sceneIt = mScenes.begin(); 
         sceneIt != mScenes.end();
         ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        if( scene && scene->isActive() )
        {
            scene->handleMouseDrag(event);
        }
    }
}

void OculonApp::mouseUp( MouseEvent event)
{
    for (SceneList::iterator sceneIt = mScenes.begin(); 
         sceneIt != mScenes.end();
         ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        if( scene && scene->isActive() )
        {
            scene->handleMouseUp(event);
        }
    }
}

void OculonApp::keyDown( KeyEvent event )
{
    switch( event.getCode() )
    {            
        // toggle pause-all
        case KeyEvent::KEY_p:
        {
            for (SceneList::iterator sceneIt = mScenes.begin(); 
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
        
        // fullscreen
        case KeyEvent::KEY_f:
            setPresentationMode( !mIsPresentationMode );
            break;

        // info panel
        case KeyEvent::KEY_SLASH:
            mInfoPanel.toggleState();
            break;
            
        // scene toggle
        case KeyEvent::KEY_1:
        case KeyEvent::KEY_2:
        case KeyEvent::KEY_3:
        case KeyEvent::KEY_4:
        case KeyEvent::KEY_5:
        case KeyEvent::KEY_6:
        case KeyEvent::KEY_7:
        case KeyEvent::KEY_8:
            {
                char index = event.getChar() - '1';
                
                if( index < mScenes.size() && mScenes[index] != NULL )
                {
                    mScenes[index]->toggleActiveVisible();
                }
            }
            break;
            
        // video capture
        case KeyEvent::KEY_r:
            if( mIsCapturingVideo )
            {
                stopVideoCapture();
            }
            else
            {
                bool useDefaultSettings = event.isShiftDown() ? false : true;
                startVideoCapture(useDefaultSettings);
            }
            break;
        case KeyEvent::KEY_ESCAPE:
            quit();
            break;
        default:
        {
            for (SceneList::iterator sceneIt = mScenes.begin(); 
                 sceneIt != mScenes.end();
                 ++sceneIt )
            {
                Scene* scene = (*sceneIt);
                if( scene && scene->isActive() )
                {
                    if( scene->handleKeyDown(event) )
                    {
                        break;
                    }
                }
            }
        }
            break;
    }
}

void OculonApp::update()
{
    char buf[64];
    snprintf(buf, 64, "%.2ffps", getAverageFps());
    mInfoPanel.addLine( buf, Color(0.5f, 0.5f, 0.5f) );
    snprintf(buf, 64, "%.1fs", getElapsedSeconds());
    mInfoPanel.addLine( buf, Color(0.5f, 0.5f, 0.5f) );
    if( mIsCapturingVideo )
    {
        mInfoPanel.addLine( "RECORDING", Color(0.9f,0.5f,0.5f) );
    }
    
    mElapsedSecondsThisFrame = getElapsedSeconds() - mLastElapsedSeconds;
    mLastElapsedSeconds = getElapsedSeconds();
    
    mAudioInput.update();
    mMidiInput.update();
    if( mEnableMindWave )
    {
        mMindWave.update();
    }
    
    if( mEnableOscServer )
    {
        mOscServer.update();
    }
    
    for (SceneList::iterator sceneIt = mScenes.begin(); 
         sceneIt != mScenes.end();
         ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        if( scene && scene->isActive() )
        {
            scene->update(mElapsedSecondsThisFrame);
        }
    }
    
    mInfoPanel.update();
}

void OculonApp::draw()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // 3D scenes
    glPushMatrix();
    {
        // setup camera
        if( mUseMayaCam )
        {
            gl::setMatrices( mMayaCam.getCamera() );
        }
        else 
        {
            gl::setMatrices( mCam );            
        }

        // enable depth buffer
        gl::enableDepthRead();
        gl::enableDepthWrite();

        // render scenes
        for (SceneList::iterator sceneIt = mScenes.begin(); 
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
    
    // capture video
	if( mIsCapturingVideo && mMovieWriter )
    {
		mMovieWriter.addFrame( copyWindowSurface(), (float)mElapsedSecondsThisFrame );
    }
    
    // debug
    drawInfoPanel();
    if( !mIsPresentationMode )
    {
        //params::InterfaceGl::draw();
        mParams.draw();
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
    if( enabled )
        hideCursor();
    else
        showCursor();
}

void OculonApp::setCamera( const Vec3f& eye, const Vec3f& look, const Vec3f& up )
{
    mCam.lookAt( eye, look, up );
}

void OculonApp::startVideoCapture(bool useDefaultPath)
{
    fs::path outputPath;
    qtime::MovieWriter::Format mwFormat;
    bool ready = false;
    
    // spawn file dialog
    // outputPath = getSaveFilePath();
    
    format filenameFormat("~/Desktop/oculon%03d.mov");
    string pathString;
    int counter = 0;
    
    while( !ready && counter < 999 )
    {
        pathString = str( filenameFormat % counter );
        outputPath = expandPath( fs::path(pathString) );
        if( ! fs::exists(outputPath) )
        {
            ready = true;
            break;
        }
        ++counter;
    }
    
    if( ready )
    {
        if( useDefaultPath )
        {
            // H.264, 30fps, high detail
            mwFormat.setCodec(1635148593); // get this value from the output of the dialog
            mwFormat.setTimeScale(3000);
            mwFormat.setDefaultDuration(1.0f/15.0f);
            mwFormat.setQuality(0.99f);
        }
        else
        {
            ready = qtime::MovieWriter::getUserCompressionSettings( &mwFormat );
        }
    }
    
    if( ready )
    {
        console() << "[main] start video capture" << "\n\tFile: " << outputPath.string() << "\n\tFramerate: " << (1.0f / mwFormat.getDefaultDuration()) << "\n\tQuality: " << mwFormat.getQuality() << std::endl;
        mMovieWriter = qtime::MovieWriter( outputPath, getWindowWidth(), getWindowHeight(), mwFormat );
        mIsCapturingVideo = true;
    }
}

void OculonApp::stopVideoCapture()
{
    console() << "[main] stop video capture\n";
    mMovieWriter.finish();
    mIsCapturingVideo = false;
}

CINDER_APP_BASIC( OculonApp, RendererGl(RendererGl::AA_MSAA_8) )
