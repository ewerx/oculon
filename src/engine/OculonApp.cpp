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
#include "Orbiter.h"
#include "Magnetosphere.h"
#include "AudioTest.h"
#include "MindWaveTest.h"

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
    
    // debug
	//glDisable( GL_TEXTURE_2D );
    
    mLastElapsedSeconds = getElapsedSeconds();
    mElapsedSecondsThisFrame = 0.0f;
    
    setupScenes();
}

void OculonApp::setupScenes()
{
    console() << "[main] creating scenes...\n";
    
    mScenes.clear(); 
    
    //TODO: serialization
    // Orbiter
    console() << "\tOrbiter\n";
    Scene* scene = new Orbiter();
    scene->init(this);
    //scene->toggleActiveVisible(); // enable
    mScenes.push_back( scene );
    
    // Magnetosphere
    console() << "\tMagneto\n";
    scene = new Magnetosphere();
    scene->init(this);
    mScenes.push_back(scene);
    
    // AudioTest
    console() << "\tAudioTest\n";
    scene = new AudioTest();
    scene->init(this);
    mScenes.push_back( scene );
    
    if( mEnableMindWave )
    {
        console() << "\tMindWave\n";
        scene = new MindWaveTest();
        scene->init(this);
        scene->toggleActiveVisible();
        mScenes.push_back(scene);
    }
    
    for (SceneList::iterator sceneIt = mScenes.begin(); 
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
}

void OculonApp::mouseDrag( MouseEvent event )
{
    if( mUseMayaCam )
    {
        // let the camera handle the interaction
        mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
    }
}

void OculonApp::keyDown( KeyEvent event )
{
    switch( event.getChar() )
    {            
        // toggle pause-all
        case 'p':
        case 'P':
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
        case 'f':
        case 'F':
            setPresentationMode( !mIsPresentationMode );
            break;

        // info panel
        case '/':
        case '?':
            mInfoPanel.toggleState();
            break;
            
        // scene toggle
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
            break;
            
        // video capture
        case 'r':
        case 'R':
            if( mIsCapturingVideo )
            {
                stopVideoCapture();
            }
            else
            {
                bool useDefaultPath = event.isShiftDown() ? false : true;
                startVideoCapture(useDefaultPath);
            }
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
    Color infoPanelText(0.5f,0.5f,0.5f);
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
    gl::clear( Colorf(0.0f, 0.0f, 0.0f) );
    
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
        // enable the depth buffer (after all, we are doing 3D)
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

CINDER_APP_BASIC( OculonApp, RendererGl(RendererGl::AA_MSAA_4) )