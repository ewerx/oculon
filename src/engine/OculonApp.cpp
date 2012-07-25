/*
 *  OculonApp.h
 *  OculonApp
 *
 *  Created by Ehsan on 11-10-16.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "cinder/gl/gl.h"
#include "cinder/gl/TileRender.h"
#include "cinder/gl/Fbo.h"
#include "cinder/audio/Input.h"
#include "cinder/Camera.h"
#include "cinder/params/Params.h"
#include "cinder/Filesystem.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"
#include <iostream>
#include <vector>
#include <boost/format.hpp>

#include "OculonApp.h"
#include "AudioInput.h"
#include "InfoPanel.h"
#include "Utils.h"
#include "Interface.h"
#include "Constants.h"

// scenes
#include "Orbiter.h"
#include "Magnetosphere.h"
#include "Pulsar.h"
#include "Binned.h"
#include "Graviton.h"
#include "Tectonic.h"
#include "Sol.h"
#include "Catalog.h"
// test scenes
#include "AudioSignal.h"
#include "MindWaveTest.h"
#include "MovieTest.h"
#include "ShaderTest.h"
#include "KinectTest.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace boost;

void OculonApp::prepareSettings( Settings *settings )
{
    mConfig.loadXML( loadFile("/Volumes/cruxpod/oculondata/params/settings.xml") );
    
	settings->setWindowSize( mConfig.getInt("window_width"), mConfig.getInt("window_height") );
	settings->setFrameRate( 60.0f );
	settings->setFullScreen( false );
    settings->enableSecondaryDisplayBlanking(false);
    
    mSetupScenesOnStart = true;
    mIsPresentationMode = true;
    mIsSendingFps       = true;
    
    mUseMayaCam         = true;
    
    mEnableSyphonServer = mConfig.getBool("syphon");;
    mDrawToScreen       = true;
    mDrawOnlyLastScene  = true;
    
    mOutputMode         = OUTPUT_FBO;
    const int outputMode = mConfig.getInt("output_mode");
    if( outputMode < OUTPUT_COUNT )
    {
        mOutputMode = (eOutputMode)outputMode;
    }
    
    mEnableOscServer    = true;
    mEnableKinect       = mConfig.getBool("kinect_enabled");
    mEnableMindWave     = mConfig.getBool("mindwave_enabled");;
}

void OculonApp::setup()
{
    console() << "[main] initializing...\n";
    
    mFpsSendTimer = 0.0f;
    
    // render
    const int fboWidth = mConfig.getInt("capture_width");
    const int fboHeight = mConfig.getInt("capture_height");
    gl::Fbo::Format format;
    format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
    mFbo = gl::Fbo( fboWidth, fboHeight, format );
    
    gl::enableVerticalSync(true);
    //wireframe
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    mLastActiveScene    = -1;
    
    // capture
    mFrameCaptureCount = 0;
    mIsCapturingVideo = false;
    mIsCapturingFrames = false;
    mCaptureDuration = 60.0f;
    mSaveNextFrame = false;
    mCaptureDebugOutput = false;
    
    // setup the default camera, looking down the z-axis
	CameraPersp cam;
    //cam.lookAt( Vec3f( 0.0f, 0.0f, 750.0f ), Vec3f::zero(), Vec3f(0.0f, 1.0f, 0.0f) );
    cam.setEyePoint( Vec3f(0.0f, 0.0f, 750.0f) );
	cam.setCenterOfInterestPoint( Vec3f::zero() );
	cam.setPerspective( 45.0f, getWindowAspectRatio(), 1.0f, 200000.0f );
    mMayaCam.setCurrentCam( cam );
    
    // params
    mParams = params::InterfaceGl( "Parameters", Vec2i( 350, getWindowHeight()*0.8f ) );
    mParams.hide();
    //mParams.setOptions("","position='10 600'");
    setupInterface();

    
    // audio input
    mAudioInput.setup();
    
    if( mEnableMidi )
    {
        mMidiInput.setup();
    }
 
    if( mEnableOscServer )
    {
        mOscServer.setup( mConfig );
    }
    
    if( mEnableMindWave )
    {
        mMindWave.setup();
    }
    
    if( mEnableKinect )
    {
        mKinectController.setup();
    }
    
    // syphon
    mScreenSyphon.setName("oculon");
    
    mLastElapsedSeconds = getElapsedSeconds();
    mElapsedSecondsThisFrame = 0.0f;
    
    console() << "[main] vsync: " << (gl::isVerticalSyncEnabled() ? "on" : "off") << std::endl;
    
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

void OculonApp::setupInterface()
{
    mThumbnailControls.clear();
    mInterface = new Interface(this, &mOscServer);
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("OCULON");
    mInterface->addButton(CreateTriggerParam("sync", NULL)
                          .oscReceiver("master", "sync"))->registerCallback( this, &OculonApp::syncInterface );
    mInterface->addEnum(CreateEnumParam("Output Mode", (int*)&mOutputMode)
                        .maxValue(OUTPUT_COUNT))->registerCallback( this, &OculonApp::onOutputModeChange );
    mInterface->addParam(CreateBoolParam("Syphon", &mEnableSyphonServer)
                         .oscReceiver("master", "syphon"));
    mInterface->addParam(CreateBoolParam("Draw FBOs", &mDrawToScreen));
    mInterface->gui()->addSeparator();
    mInterface->addParam(CreateBoolParam("Capture Debug", &mCaptureDebugOutput));
    mInterface->addParam(CreateBoolParam("Capture Frames", &mIsCapturingFrames))->registerCallback( this, &OculonApp::onFrameCaptureToggle );
    mInterface->addParam(CreateFloatParam("Capture Duration", &mCaptureDuration)
                         .minValue(1.0f)
                         .maxValue(600.0f));
}

bool OculonApp::syncInterface()
{
    mInterface->sendAll();
    for (SceneList::iterator sceneIt = mScenes.begin(); 
         sceneIt != mScenes.end();
         ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        scene->mInterface->sendAll();
    }
    return false;
}

void OculonApp::addScene(Scene* scene, bool autoStart)
{
    console() << (mScenes.size()+1) << ": " << scene->getName() << std::endl;
    
    scene->init(this);
    if( mSetupScenesOnStart )
    {
        scene->setup();
    }
    
    if( autoStart )
    {
        scene->setRunning(true);
        scene->setVisible(true);
        scene->setDebug(true);
    }
    mScenes.push_back(scene);
    
    // interface
    mInterface->gui()->addColumn();
    mInterface->gui()->addButton(scene->getName())->registerCallback( boost::bind( &OculonApp::showInterface, this, mScenes.size()-1) );
    // scene thumbnails
    if( scene->getFbo() )
    {
        mThumbnailControls.push_back( mInterface->gui()->addParam(scene->getName(), &(scene->getFbo().getTexture())) );
    }
    mInterface->gui()->addButton("toggle")->registerCallback( boost::bind( &OculonApp::toggleScene, this, mScenes.size()-1) );
    mInterface->addParam(CreateBoolParam("on", &(scene->mIsVisible)))->registerCallback( scene, &Scene::setRunningByVisibleState );
    mInterface->addParam(CreateBoolParam("debug", &(scene->mIsDebug)))->registerCallback( scene, &Scene::onDebugChanged );
}

bool OculonApp::toggleScene(const int sceneId)
{
    if( sceneId >= 0 && sceneId < mScenes.size() )
    {
        mScenes[sceneId]->toggleActiveVisible();
        if( mScenes[sceneId]->isVisible() )
        {
            mLastActiveScene = sceneId;
        }
    }
    
    return false;
}

bool OculonApp::showInterface(const int sceneId)
{
    if( sceneId < 0 )
    {
        // hide all scene interfaces
        for (SceneList::iterator sceneIt = mScenes.begin(); 
             sceneIt != mScenes.end();
             ++sceneIt )
        {
            Scene* scene = (*sceneIt);
            scene->showInterface(false);
        }
        mInterface->gui()->setEnabled( (sceneId == INTERFACE_MAIN) );
    }
    else if( sceneId < mScenes.size() )
    {
        mScenes[sceneId]->showInterface(true);
        mInterface->gui()->setEnabled(false);
        mLastActiveScene = sceneId;
    }
    
    return false;
}

//MARK: Setup Scenes
void OculonApp::setupScenes()
{
    console() << "[main] creating scenes...\n";
    
    mScenes.clear(); 
    
    if( mConfig.getBool("orbiter") )    addScene( new Orbiter() );
    if( mConfig.getBool("binned") )     addScene( new Binned() );
    if( mConfig.getBool("pulsar") )     addScene( new Pulsar() );
    if( mConfig.getBool("magneto") )    addScene( new Magnetosphere() );
    if( mConfig.getBool("graviton") )   addScene( new Graviton() );
    if( mConfig.getBool("tectonic") )   addScene( new Tectonic() );
    if( mConfig.getBool("sol") )        addScene( new Sol() );
    if( mConfig.getBool("catalog") )    addScene( new Catalog() );
    if( mConfig.getBool("audio") )      addScene( new AudioSignal() );
    
    // Test Scenes
    //addScene( new MovieTest() );
    //addScene( new ShaderTest() );
    if( mEnableKinect )
    {
        if( mConfig.getBool("kinect_test") ) addScene( new KinectTest() );
    }
    if( mEnableMindWave )
    {
        if( mConfig.getBool("mindwave_test") ) addScene( new MindWaveTest() );
    }
}

void OculonApp::resize( ResizeEvent event )
{
    CameraPersp cam = mMayaCam.getCamera();
    cam.setAspectRatio( getWindowAspectRatio() );
    mMayaCam.setCurrentCam( cam );
    
    int index = 0;
    for (SceneList::iterator sceneIt = mScenes.begin(); 
        sceneIt != mScenes.end();
        ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        if( scene && scene->isRunning() )
        {
            scene->resize();
            if( scene->getFbo() )
            {
                mThumbnailControls[index]->resetTexture( &(scene->getFbo().getTexture()) );
            }
        }
        ++index;
    }
}

void OculonApp::mouseMove( MouseEvent event )
{    
    for (SceneList::iterator sceneIt = mScenes.begin(); 
         sceneIt != mScenes.end();
         ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        if( scene && scene->isRunning() )
        {
            scene->handleMouseMove(event);
        }
    }
}

void OculonApp::mouseDown( MouseEvent event )
{
    // let the camera handle the interaction
    mMayaCam.mouseDown( event.getPos() );
    
    for (SceneList::iterator sceneIt = mScenes.begin(); 
         sceneIt != mScenes.end();
         ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        if( scene && scene->isRunning() )
        {
            scene->handleMouseDown(event);
        }
    }
}

void OculonApp::mouseDrag( MouseEvent event )
{
    // let the camera handle the interaction
    mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );

    
    for (SceneList::iterator sceneIt = mScenes.begin(); 
         sceneIt != mScenes.end();
         ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        if( scene && scene->isRunning() )
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
        if( scene && scene->isRunning() )
        {
            scene->handleMouseUp(event);
        }
    }
}

void OculonApp::keyDown( KeyEvent event )
{
    bool passToScenes = false;
    
    switch( event.getCode() )
    {            
        // toggle pause-all
        case KeyEvent::KEY_p:
        {
            if( event.isShiftDown() )
            {
                mParams.show( !mParams.isVisible() );
            }
            else
            {
                for (SceneList::iterator sceneIt = mScenes.begin(); 
                     sceneIt != mScenes.end();
                     ++sceneIt )
                {
                    Scene* scene = (*sceneIt);
                    assert( scene != NULL );
                    if( scene )
                    {
                        scene->setRunning( !scene->isRunning() );
                    }
                }
            }
            break;
        }
            
        // fullscreen
        case KeyEvent::KEY_f:
            toggleFullscreen();
            break;
            
        case KeyEvent::KEY_i:
            setPresentationMode( !mIsPresentationMode );
            break;

        // info panel
        case KeyEvent::KEY_SLASH:
            mInfoPanel.toggleState();
            break;
            
        case KeyEvent::KEY_0:
            showInterface(INTERFACE_MAIN);
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
                char index = event.getCode() - KeyEvent::KEY_1;
                
                if( index < mScenes.size() && mScenes[index] != NULL )
                {
                    if( event.isControlDown() )
                    {
                        mScenes[index]->setVisible( !mScenes[index]->isVisible() );
                    }
                    else if( event.isMetaDown() )
                    {
                        mScenes[index]->setRunning( !mScenes[index]->isRunning() );
                    }
                    else if( event.isAltDown() )
                    {
                        mScenes[index]->setDebug( !mScenes[index]->isDebug() );
                    }
                    else if( event.isShiftDown() )
                    {
                        showInterface(index);
                    }
                    else
                    {
                        mScenes[index]->toggleActiveVisible();
                        if( mScenes[index]->isVisible() )
                        {
                            mLastActiveScene = index;
                        }
                    }
                }
            }
            break;
            
        // video capture
        case KeyEvent::KEY_r:
            if( event.isShiftDown() )
            {
                if( mIsCapturingVideo )
                {
                    stopVideoCapture();
                }
                else
                {
                    bool useDefaultSettings = event.isShiftDown() ? false : true;
                    startVideoCapture(useDefaultSettings);
                }
            }
            else
            {
                enableFrameCapture( !mIsCapturingFrames );
            }
            break;
        case KeyEvent::KEY_c:
            if( event.isShiftDown() )
            {
                const Camera& cam = mMayaCam.getCamera();
                
                console() << "Camera (manual):" << std::endl;
                console() << "\tEye: " << cam.getEyePoint() << " LookAt: " << cam.getViewDirection() << " Up: " << cam.getWorldUp() << std::endl;
            }
            else
            {
                passToScenes = true;
            }
            break;
        case KeyEvent::KEY_s:
            if( event.isShiftDown() )
            {
                mSaveNextFrame = true;
            }
            else if( event.isControlDown() )
            {
                mEnableSyphonServer = !mEnableSyphonServer;
            }
            else
            {
                passToScenes = true;
            }
            break;
        case KeyEvent::KEY_o://pass-thru
        case KeyEvent::KEY_b:
            mScenes[SCENE_ORBITER]->handleKeyDown(event);
            mScenes[SCENE_BINNED]->handleKeyDown(event);
            break;
        case KeyEvent::KEY_ESCAPE:
            quit();
            break;
        default:
            passToScenes = true;
            break;
    }
    
    if( passToScenes )
    {
        for (SceneList::iterator sceneIt = mScenes.begin(); 
             sceneIt != mScenes.end();
             ++sceneIt )
        {
            Scene* scene = (*sceneIt);
            if( scene && scene->isRunning() )
            {
                if( scene->handleKeyDown(event) )
                {
                    break;
                }
            }
        }
    }
}

void OculonApp::update()
{
    mElapsedSecondsThisFrame = getElapsedSeconds() - mLastElapsedSeconds;
    mLastElapsedSeconds = getElapsedSeconds();
    
    const int BUFSIZE = 256;
    char buf[BUFSIZE];
    const float fps = getAverageFps();
    snprintf(buf, BUFSIZE, "%.2ffps", fps);
    
    if( mIsSendingFps )
    {
        mFpsSendTimer += mElapsedSecondsThisFrame;
        if( mFpsSendTimer >= 1.0f )
        {
            mFpsSendTimer = 0.0f;
            osc::Message message;
            message.setAddress("/oculon/master/fps");
            message.addFloatArg(fps);
            mOscServer.sendMessage(message, OscServer::DEST_INTERFACE, LOGLEVEL_SILENT);
        }
    }
    
    if( mInfoPanel.isVisible() )
    {
        Color defaultColor(0.5f, 0.5f, 0.5f);
        Color fpsColor(0.5f, 0.5f, 0.5f);
        if( fps < 20.0f )
        {
            fpsColor = Color(1.0f, 0.25f, 0.25f);
        }
        else if( fps < 30.0f )
        {
            fpsColor = Color(0.85f, 0.75f, 0.05f);
        }
        mInfoPanel.addLine( buf, fpsColor );
        snprintf(buf, BUFSIZE, "%.1fs", mLastElapsedSeconds);
        mInfoPanel.addLine( buf, Color(0.5f, 0.5f, 0.5f) );
        switch( mOutputMode )
        {
            case OUTPUT_DIRECT:
                mInfoPanel.addLine( "DIRECT", defaultColor );
                break;
            case OUTPUT_FBO:
                mInfoPanel.addLine( "FBO", defaultColor );
                break;
            case OUTPUT_MULTIFBO:
                mInfoPanel.addLine( "MULTI-FBO", defaultColor );
                break;
            default:
                break;
        }
        snprintf(buf, BUFSIZE, "%d x %d", getViewportWidth(), getViewportHeight());
        mInfoPanel.addLine( buf, defaultColor );
        
        if( mEnableSyphonServer )
        {
            mInfoPanel.addLine( "SYPHON", Color(0.3f, 0.3f, 1.0f) );
        }
        
        if( mIsCapturingVideo )
        {
            mInfoPanel.addLine( "RECORDING", Color(0.9f,0.5f,0.5f) );
        }
        
        if( mIsCapturingFrames )
        {
            snprintf(buf, BUFSIZE, "CAPTURING #%d", mFrameCaptureCount);
            mInfoPanel.addLine( buf, Color(0.9f,0.5f,0.5f) );
            
            snprintf(buf, BUFSIZE, "-- duration: %.1fs / %.1fs", mFrameCaptureCount/kCaptureFramerate, mCaptureDuration);
            mInfoPanel.addLine( buf, Color(0.9f,0.5f,0.5f) );
            
            float time = (mCaptureDuration*kCaptureFramerate - mFrameCaptureCount) * mElapsedSecondsThisFrame;
            if( time > 60.f )
            {
                const int min = time / 60;
                snprintf(buf, BUFSIZE, "-- finish in %dm%ds", min, mod((int)time,60));
            }
            else
            {
                snprintf(buf, BUFSIZE, "-- finish in %.0fs", time);
            }
            mInfoPanel.addLine( buf, Color(0.9f,0.5f,0.5f) );
        }
    }

    if( mIsCapturingFrames )
    {
        const float elapsed = (mFrameCaptureCount/kCaptureFramerate);
        if( elapsed >= mCaptureDuration )
        {
            enableFrameCapture(false);
        }
    }
    
    mAudioInput.update();
    
    if( mEnableMidi )
    {
        mMidiInput.update();
    }
    if( mEnableMindWave )
    {
        mMindWave.update();
    }
    
    if( mEnableOscServer )
    {
        mOscServer.update();
    }
    
    if( mEnableKinect )
    {
        mKinectController.update();
    }
    
    // update scenes
    const float dt = mIsCapturingFrames ? (1.0f/kCaptureFramerate) : mElapsedSecondsThisFrame;
    for (SceneList::iterator sceneIt = mScenes.begin(); 
         sceneIt != mScenes.end();
         ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        if( scene )
        {
            Color labelColor = Color(0.5f,0.5f,0.5f);
            if( scene->isRunning() )
            {
                if( !scene->isSetup() )
                {
                    scene->setup();
                }
                scene->update(dt);
                labelColor = Color(0.75f, 0.4f, 0.4f);
            }
            mInfoPanel.addLine(scene->getName(), labelColor);
        }
    }
    
    mInterface->update();
    mInfoPanel.update();
}

void OculonApp::drawToScreen()
{
    if( mOutputMode == OUTPUT_MULTIFBO )
    {
        renderScenes();
    }
    
    if( mOutputMode != OUTPUT_MULTIFBO || mDrawToScreen )
    {
        gl::disableDepthRead();
        gl::disableDepthWrite();
        gl::enableAlphaBlending();
        gl::clear( ColorA(0.0f,0.0f,0.0f,1.0f) );
        gl::color( ColorA(1.0f,1.0f,1.0f,1.0f) );
        drawScenes();
    }
}

void OculonApp::drawToFbo()
{
    Area prevViewport = gl::getViewport();
    // bind the framebuffer - now everything we draw will go there
    mFbo.bindFramebuffer();
    
    gl::disableDepthRead();
    gl::disableDepthWrite();
    gl::disableAlphaBlending();
    
    // setup the viewport to match the dimensions of the FBO
    gl::setViewport( mFbo.getBounds() );
    gl::clear( Color(0.0f,0.0f,0.0f) );
    
    drawScenes();
    
    mFbo.unbindFramebuffer();
    
    gl::setViewport( prevViewport );
}

void OculonApp::drawFromFbo()
{
    //gl::enableDepthRead();
    //gl::enableDepthWrite();
    //gl::disableAlphaBlending();
    //glEnable(GL_TEXTURE_2D);
    // draw the captured texture back to screen
    float width = getWindowWidth();
    float height = getWindowHeight();
    gl::color( ColorA(1.0f,1.0f,1.0f,1.0f) );
    gl::setMatricesWindow( Vec2i( width, height ) );
    
    // flip
    gl::draw( mFbo.getTexture(), Rectf( 0, height, width, 0 ) );

}

void OculonApp::renderScenes()
{
    // render scenes to FBO
    for (SceneList::iterator sceneIt = mScenes.begin(); 
         sceneIt != mScenes.end();
         ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        assert( scene != NULL );
        if( scene && scene->isVisible() )
        {
            scene->drawToFbo();
        }
    }

}

void OculonApp::drawScenes()
{
    if( mOutputMode == OUTPUT_MULTIFBO )
    {
        // draw scene FBO textures
        gl::disableDepthRead();
        gl::disableDepthWrite();
        gl::disableAlphaBlending();
        glEnable(GL_TEXTURE_2D);
        gl::color( ColorA(1.0f,1.0f,1.0f,1.0f) );
        if( mDrawOnlyLastScene )
        {
            if( mLastActiveScene >= 0 && mLastActiveScene < mScenes.size() )
            {
                Scene* scene = mScenes[mLastActiveScene];
                if( scene && scene->isVisible() )
                {
                    gl::Fbo& fbo = scene->getFbo();
                    gl::draw( fbo.getTexture(), Rectf( 0, fbo.getHeight(), fbo.getWidth(), 0 ) );
                }
            }
        }
        else
        {
            for (SceneList::iterator sceneIt = mScenes.begin(); 
                 sceneIt != mScenes.end();
                 ++sceneIt )
            {
                Scene* scene = (*sceneIt);
                assert( scene != NULL );
                if( scene && scene->isVisible() )
                {
                    gl::Fbo& fbo = scene->getFbo();
                    gl::draw( fbo.getTexture(), Rectf( 0, fbo.getHeight(), fbo.getWidth(), 0 ) );
                }
            }
        }
    }
    else
    {
        // draw scenes directly
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
}

void OculonApp::draw()
{
    gl::clear();
    
    if( mOutputMode == OUTPUT_FBO )
    {
        drawToFbo();
    }
    else
    {
        drawToScreen();
        
        if( mCaptureDebugOutput ) 
        {
            drawDebug();
        }
    }
    
    // capture video
	if( mIsCapturingVideo && mMovieWriter )
    {
		mMovieWriter.addFrame( copyWindowSurface(), (float)mElapsedSecondsThisFrame );
    }
    
    // capture frames
    if( mIsCapturingFrames )
    {
        captureFrames();
    }
    
    // screenshot
    if( mSaveNextFrame )
    {
        saveScreenshot();
	}
    
    if( mOutputMode == OUTPUT_FBO )
    {
        drawFromFbo();
    }
    
    if( mEnableSyphonServer )
    {
        switch( mOutputMode )
        {
            case OUTPUT_DIRECT:
                mScreenSyphon.publishScreen();
                break;
                
            case OUTPUT_FBO:
                mScreenSyphon.publishTexture(&mFbo.getTexture());
                break;
                
            case OUTPUT_MULTIFBO:
                // publish each scene separately
                for (SceneList::iterator sceneIt = mScenes.begin(); 
                     sceneIt != mScenes.end();
                     ++sceneIt )
                {
                    Scene* scene = (*sceneIt);
                    assert( scene != NULL );
                    if( scene && scene->isVisible() )
                    {
                        scene->publishToSyphon();
                    }
                }
                break;
            
            default:
                break;
        }
    }
    
    if( !mCaptureDebugOutput )
    {
        drawDebug();
    }
}

void OculonApp::drawDebug()
{
    gl::pushMatrices();
    gl::setMatricesWindow( Vec2i( getWindowWidth(), getWindowHeight() ) );
    for (SceneList::iterator sceneIt = mScenes.begin(); 
         sceneIt != mScenes.end();
         ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        assert( scene != NULL );
        if( scene )
        {
            scene->drawInterface();
            if( scene->isVisible() && scene->isDebug() )
            {
                scene->drawDebug();
            }
        }
    }
    gl::popMatrices();
    
    if( mInfoPanel.isVisible() )
    {
        mInfoPanel.render( Vec2f( getWindowWidth(), getWindowHeight() ) );
    }
    
    if( !mIsPresentationMode )
    {
        mInterface->draw();
        params::InterfaceGl::draw();
        mParams.draw();
    }
}

#pragma MARK Capture

void OculonApp::captureFrames()
{
    ++mFrameCaptureCount;
    
    if( mOutputMode == OUTPUT_FBO )
    {
        writeImage( mFrameCapturePath + "/" + Utils::leftPaddedString( toString(mFrameCaptureCount) ) + ".png", mFbo.getTexture() );
    }
    else
    {
        writeImage( mFrameCapturePath + "/" + Utils::leftPaddedString( toString(mFrameCaptureCount) ) + ".png", copyWindowSurface() );
    }
}

void OculonApp::saveScreenshot()
{
    mSaveNextFrame = false;
    fs::path p = Utils::getUniquePath( "~/Desktop/oculon_screenshot.png" );
    if( mOutputMode == OUTPUT_FBO )
    {
        writeImage( p, mFbo.getTexture() );
    }
    else
    {
        writeImage( p, copyWindowSurface() );
    }
}

void OculonApp::setPresentationMode( bool enabled )
{
    //mInfoPanel.setVisible(!enabled);
    mIsPresentationMode = enabled;
    showInterface( mIsPresentationMode ? INTERFACE_NONE : INTERFACE_MAIN );
}

void OculonApp::toggleFullscreen()
{
    setFullScreen( !isFullScreen() );
    if( isFullScreen() )
        hideCursor();
    else
        showCursor();
}

void OculonApp::startVideoCapture(bool useDefaultPath)
{
    fs::path outputPath = Utils::getUniquePath("~/Desktop/oculon_video.mov");
    qtime::MovieWriter::Format mwFormat;
    bool ready = false;
    
    // spawn file dialog
    // outputPath = getSaveFilePath();
    
    if( useDefaultPath )
    {
        // H.264, 30fps, high detail
        mwFormat.setCodec(1635148593); // get this value from the output of the dialog
        mwFormat.setTimeScale(3000);
        mwFormat.setDefaultDuration(1.0f/15.0f);
        mwFormat.setQuality(0.99f);
        ready = true;
    }
    else
    {
        // user prompt
        ready = qtime::MovieWriter::getUserCompressionSettings( &mwFormat );
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

void OculonApp::enableFrameCapture( const bool enable )
{
    mIsCapturingFrames = enable;
    onFrameCaptureToggle();
}

bool OculonApp::onFrameCaptureToggle()
{
    if( mIsCapturingFrames )
    {
        mFrameCaptureCount = 0;
        fs::path outputPath = Utils::getUniquePath("/Volumes/cruxpod/oculondata/capture/oculon_capture");
        mFrameCapturePath = outputPath.string();
        if( fs::create_directory(outputPath) )
        {
            console() << "[main] frame capture started. location: " << mFrameCapturePath << std::endl;
        }
        else
        {
            mIsCapturingFrames = false;
            console() << "[main] ERROR: frame capture failed to start. location: " << mFrameCapturePath << std::endl;
        }
    }
    else
    {
        console() << "[main] frame capture stopped" << std::endl;
    }
    
    return false;
}

bool OculonApp::onOutputModeChange()
{
    if( mOutputMode == OUTPUT_FBO )
    {
        // uncomment for high-res capture
        //setWindowSize( 860, 860 );
        resize( ResizeEvent(mFbo.getSize()) );
    }
    else
    {
        resize( ResizeEvent(getWindowSize()) );
    }
    
    return false;
}

int OculonApp::getViewportWidth() const
{
    if( mOutputMode == OUTPUT_FBO ) 
    {
        return mFbo.getWidth();
    }
    else
    {
        return getWindowWidth();
    }
}

int OculonApp::getViewportHeight() const
{
    if( mOutputMode == OUTPUT_FBO ) 
    {
        return mFbo.getHeight();
    }
    else
    {
        return getWindowHeight();
    }
}

Area OculonApp::getViewportBounds() const
{
    if( mOutputMode == OUTPUT_FBO ) 
    {
        return Area( 0, 0, mFbo.getWidth(), mFbo.getHeight() );
    }
    else
    {
        return getWindowBounds();
    }
}

Vec2i OculonApp::getViewportSize() const
{
    if( mOutputMode == OUTPUT_FBO )
    {
        return Vec2i( mFbo.getWidth(), mFbo.getHeight() );
    }
    else
    {
        return getWindowSize();
    }
}

float OculonApp::getViewportAspectRatio() const
{
    if( mOutputMode == OUTPUT_FBO )
    {
        return (mFbo.getWidth() / (float)mFbo.getHeight());
    }
    else
    {
        return getWindowAspectRatio();
    }
}

CINDER_APP_BASIC( OculonApp, RendererGl(0) )
