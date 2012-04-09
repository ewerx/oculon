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

// scenes
#include "Orbiter.h"
#include "Magnetosphere.h"
#include "Pulsar.h"
#include "Binned.h"
#include "Graviton.h"
// test scenes
#include "AudioTest.h"
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
	settings->setWindowSize( 1024, 768 );
	settings->setFrameRate( 60.0f );
	settings->setFullScreen( false );
    settings->enableSecondaryDisplayBlanking(false);
    
    mIsPresentationMode = false;
    mUseMayaCam         = true;
    
    mEnableMindWave     = false;
    mEnableOscServer    = true;
    mEnableSyphonServer = false;
    mEnableKinect       = false;
}

void OculonApp::setup()
{
    console() << "[main] initializing...\n";
    
    // capture
    mFrameCaptureCount = 0;
    mIsCapturingVideo = false;
    mIsCapturingFrames = false;
    mSaveNextFrame = false;
    mIsCapturingHighRes = false;
    mCaptureDebugOutput = false;
    static const int FBO_WIDTH = 2240;
    static const int FBO_HEIGHT = 2240;
    gl::Fbo::Format format;
    format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
    mFbo = gl::Fbo( FBO_WIDTH, FBO_HEIGHT, format );
    
    //wireframe
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    
    // setup the default camera, looking down the z-axis
	CameraPersp cam;
    //cam.lookAt( Vec3f( 0.0f, 0.0f, 750.0f ), Vec3f::zero(), Vec3f(0.0f, 1.0f, 0.0f) );
    cam.setEyePoint( Vec3f(0.0f, 0.0f, 750.0f) );
	cam.setCenterOfInterestPoint( Vec3f::zero() );
	cam.setPerspective( 45.0f, getWindowAspectRatio(), 1.0f, 200000.0f );
    mMayaCam.setCurrentCam( cam );
    
    // params
    mParams = params::InterfaceGl( "Parameters", Vec2i( 350, getWindowHeight()*0.8f ) );
    //mParams.setOptions("","position='10 600'");

    
    // audio input
    mAudioInput.setup();
    
    if( mEnableMindWave )
    {
        mMindWave.setup();
    }
    mMidiInput.setEnabled(false);//TODO: refactor
    
    if( mEnableOscServer )
    {
        mOscServer.setup();
    }
    
    if( mEnableKinect )
    {
        mKinectController.setup();
    }
    
    // syphon
    mScreenSyphon.setName("Oculon");
    
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
// TODO: Scenes
void OculonApp::setupScenes()
{
    console() << "[main] creating scenes...\n";
    
    mScenes.clear(); 
    
    int sceneId = 0;
    
    //TODO: serialization    
    
    const bool autoStart = true;
    
    // Orbiter
    //console() << ++sceneId << ": Orbiter\n";
    //addScene( new Orbiter(), autoStart );
    
    // Binned
    //console() << ++sceneId << ": Binned\n";
    //addScene( new Binned() );
    
    // Pulsar
    //console() << ++sceneId << ": Pulsar\n";
    //addScene( new Pulsar() );
    
    // Magnetosphere
    //console() << ++sceneId << ": Magneto\n";
    //addScene( new Magnetosphere() );
    
    // Graviton
    console() << ++sceneId << ": Graviton\n";
    addScene( new Graviton(), autoStart );
    
    // AudioTest
    console() << ++sceneId << ": AudioTest\n";
    addScene( new AudioTest() );
    
    // MovieTest
    //console() << ++sceneId << ": MovieTest\n";
    //addScene( new MovieTest() );
    
    // ShaderTest
    console() << ++sceneId << ": ShaderTest\n";
    addScene( new ShaderTest() );
    
    if( mEnableKinect )
    {
        // KinectTest
        //console() << ++sceneId << ": KinectTest\n";
        //addScene( new KinectTest(), autoStart );
    }
    
    if( mEnableMindWave )
    {
        console() << "\t" << sceneId << ": MindWave\n";
        addScene( new MindWaveTest() );
    }
}

void OculonApp::resize( ResizeEvent event )
{
    CameraPersp cam = mMayaCam.getCamera();
    cam.setAspectRatio( getWindowAspectRatio() );
    mMayaCam.setCurrentCam( cam );
    
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
    // let the camera handle the interaction
    mMayaCam.mouseDown( event.getPos() );
    
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
    // let the camera handle the interaction
    mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );

    
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
    bool passToScenes = false;
    
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
                    if( event.isControlDown() )
                    {
                        mScenes[index]->setVisible( !mScenes[index]->isVisible() );
                    }
                    else
                    {
                        mScenes[index]->toggleActiveVisible();
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
        case KeyEvent::KEY_h:
            mIsCapturingHighRes = !mIsCapturingHighRes;
            if( mIsCapturingHighRes )
            {
                resize( ResizeEvent(mFbo.getSize()) );
            }
            else
            {
                resize( ResizeEvent(getWindowSize()) );
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
            if( scene && scene->isActive() )
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
    char buf[256];
    snprintf(buf, 256, "%.2ffps", getAverageFps());
    mInfoPanel.addLine( buf, Color(0.5f, 0.5f, 0.5f) );
    snprintf(buf, 256, "%.1fs", getElapsedSeconds());
    mInfoPanel.addLine( buf, Color(0.5f, 0.5f, 0.5f) );
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
        const float videoFps = 25.0f;
        snprintf(buf, 64, "CAPTURING #%d (%ds)", mFrameCaptureCount, (int)(mFrameCaptureCount/videoFps));
        mInfoPanel.addLine( buf, Color(0.9f,0.5f,0.5f) );
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
    
    if( mEnableKinect )
    {
        mKinectController.update();
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

void OculonApp::drawToScreen()
{
    drawScenes();
}

void OculonApp::drawToFbo()
{
    Area prevViewport = gl::getViewport();
    // bind the framebuffer - now everything we draw will go there
    mFbo.bindFramebuffer();
    
    // setup the viewport to match the dimensions of the FBO
    gl::setViewport( mFbo.getBounds() );
    gl::clear( Color(0.0f,0.0f,0.0f) );
    
    drawScenes();
    
    mFbo.unbindFramebuffer();
    
    gl::setViewport( prevViewport );
    
}

void OculonApp::drawFromFbo()
{
    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::pushMatrices();
    glEnable(GL_TEXTURE_2D);
    // draw the captured texture back to screen
    glColor4f(1.0f,1.0f,1.0f,1.0f);
    gl::setMatricesWindow( Vec2i( getWindowWidth(), getWindowHeight() ) );
    float width = getWindowWidth();
    float height = getWindowHeight();
    gl::draw( mFbo.getTexture(0), Rectf( 0, 0, width, height ) );
    gl::popMatrices();

}

void OculonApp::drawScenes()
{
    glPushMatrix();
    {
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
}

void OculonApp::draw()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    if( mIsCapturingHighRes )
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
    
    if( mIsCapturingHighRes )
    {
        drawFromFbo();
    }
    
    if( mEnableSyphonServer )
    {    
        mScreenSyphon.publishScreen(); //publish the screen
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
        if( scene && scene->isVisible() )
        {
            scene->drawInterface();
            scene->drawDebug();
        }
    }
    gl::popMatrices();
    
    mInfoPanel.render( Vec2f( getWindowWidth(), getWindowHeight() ) );
    if( !mIsPresentationMode )
    {
        params::InterfaceGl::draw();
        mParams.draw();
    }
}

void OculonApp::captureFrames()
{
    ++mFrameCaptureCount;
    
    if( mIsCapturingHighRes )
    {
#ifdef TILED_RENDER
        //HACKHACK
        mIsCapturingFrames = false;
        
        gl::TileRender tr( 2200, 2200 );
        //CameraPersp cam;
        //cam.lookAt( mCam.getEyePoint(), mCam.getPos() );
        //cam.setPerspective( 60.0f, tr.getImageAspectRatio(), 1, 20000 );
        tr.setMatricesWindowPersp( getWindowWidth(), getWindowHeight() );
        while( tr.nextTile() )
        {
            draw();
        }
        writeImage( mFrameCapturePath + "/" + Utils::leftPaddedString( toString(mFrameCaptureCount) ) + ".png", tr.getSurface() );
        
        //HACKHACK
        mIsCapturingFrames = true;
#endif
        
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
    if( mIsCapturingHighRes )
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
    setFullScreen(enabled);
    mInfoPanel.setVisible(!enabled);
    mIsPresentationMode = enabled;
    if( enabled )
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

void OculonApp::enableFrameCapture( bool enable )
{
    mIsCapturingFrames = enable;
    
    if( mIsCapturingFrames )
    {
        mFrameCaptureCount = 0;
        fs::path outputPath = Utils::getUniquePath("/Volumes/cruxpod/capture/oculon_capture");
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
}

int OculonApp::getViewportWidth() const
{
    if( mIsCapturingHighRes ) 
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
    if( mIsCapturingHighRes ) 
    {
        return mFbo.getHeight();
    }
    else
    {
        return getWindowHeight();
    }
}


CINDER_APP_BASIC( OculonApp, RendererGl(RendererGl::AA_MSAA_4) )
