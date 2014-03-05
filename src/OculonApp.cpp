//
//  OculonApp.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 12-03-07.
//  Copyright (c) 2013 ewerx. All rights reserved.
//

#include "OculonApp.h"
#include "AudioInput.h"
#include "InfoPanel.h"
#include "Utils.h"
#include "Interface.h"
#include "Constants.h"

// scenes
#include "Orbiter.h"
#include "Binned.h"
#include "Graviton.h"
#include "Tectonic.h"
#include "Catalog.h"
#include "Dust.h"
#include "Flock.h"
#include "Corona.h"
#include "Trails.h"
#include "Contour.h"
#include "Grid.h"
#include "Parsec.h"
#include "TextOrbit.h"
#include "Polyhedron.h"
#include "Terrain.h"
#include "Deformer.h"
#include "Lines.h"
#include "Fluid.h"
// shader scenes
#include "Cells.h"
#include "CircleWave.h"
#include "Rings.h"
#include "Tilings.h"
#include "ObjectShaders.h"
#include "TextureShaders.h"
#include "Voronoi.h"
#include "Oscillator.h"
// test scenes
#include "AudioSignal.h"
#include "MindWaveTest.h"
//#include "MovieTest.h"
#include "ShaderTest.h"
//#include "KinectTest.h"
//#include "SkeletonTest.h" // TODO: kinect
#include "FisheyeTest.h"

#include <iostream>
#include <vector>
#include <boost/format.hpp>

#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
#include "cinder/audio/Input.h"
#include "cinder/Camera.h"
#include "cinder/params/Params.h"
#include "cinder/Filesystem.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace boost;

void OculonApp::prepareSettings( Settings *settings )
{
    
    fs::path settingsPath = expandPath( fs::path("~/Documents/oculon/settings.xml") );
    mConfig.loadXML( loadFile(settingsPath) );
    
	settings->setWindowSize( mConfig.getInt("window_width"), mConfig.getInt("window_height") );
	settings->setFrameRate( 60.0f );
	settings->setFullScreen( false );
    settings->setTitle("Oculon");
    
    mSetupScenesOnStart = true;
    mIsPresentationMode = false;
    mIsSendingFps       = true;
    
    mUseMayaCam         = true;
    
    mEnableSyphonServer = mConfig.getBool("syphon");
    mDrawToScreen       = true;
    mDrawOnlyLastScene  = true;
    mDebugRender        = false;
    mEnableManualCamControls = true;
    mBackgroundAlpha    = 1.0f;
    
    mOutputMode         = OUTPUT_FBO;
    const int outputMode = mConfig.getInt("output_mode");
    if( outputMode < OUTPUT_COUNT )
    {
        mOutputMode = (eOutputMode)outputMode;
    }
    
    mEnableMidi         = mConfig.getBool("midi_enabled");
    mEnableOscServer    = mConfig.getBool("osc_enabled");
    mEnableKinect       = mConfig.getBool("kinect_enabled");
    mEnableMindWave     = mConfig.getBool("mindwave_enabled");
    mEnableOculus       = mConfig.getBool("oculusrift");
    
    // oculus
    if( mEnableOculus )
    {
        // if there are multiple displays, assume last one is rift
        if( Display::getDisplays().size() > 1 )
        {
            settings->setDisplay( Display::getDisplays().back() );
        }
        
        settings->setFullScreen();
    }
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
    for( int layerIndex = 0; layerIndex < MAX_LAYERS; ++layerIndex )
    {
        mFbo[layerIndex] = gl::Fbo( fboWidth, fboHeight, format );
    }
    mDomeRenderer.setup( fboWidth, fboHeight );
    
    mViewportWidth = fboWidth;
    mViewportHeight = fboHeight;
    
    gl::enableVerticalSync(true);
    //wireframe
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    mLastActiveScene    = -1;
    
    mVisibleLayerIndex = 0;
    
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
    mParams = params::InterfaceGl::create( getWindow(), "Parameters", toPixels(Vec2i( 350, getWindowHeight()*0.8f )) );
    mParams->hide();
    //mParams->setOptions("","position='10 600'");
    
    // audio input
    mAudioInput.setup();
    mAudioInputHandler.setup(true);
    
    setupInterface();
    
    if( mEnableMidi )
    {
        mMidiInput.setup();
    }
    
    if( mEnableOscServer )
    {
        // TODO: decouple OSC and MIDI or create a wrapper class
        mOscServer.setup( mConfig, mEnableMidi ? &mMidiInput : NULL );
        mAudioInput.setupLiveGrabberInput(mOscServer);
    }
    
    if( mEnableMindWave )
    {
        mMindWave.setup();
    }
    
    if( mEnableKinect )
    {
//        mKinectController.setup(); // TODO: update kinect setup
    }
    
    if( mEnableSyphonServer )
    {
        if( mOutputMode == OUTPUT_FBO )
        {
            // syphon
            char nameBuf[256];
            for( int layerIndex = 0; layerIndex < MAX_LAYERS; ++layerIndex )
            {
                snprintf(nameBuf, 256, "oculon-%d", layerIndex+1);
                mScreenSyphon[layerIndex].setName(nameBuf);
            }
        }
        else
        {
            mScreenSyphon[0].setName("DEBUG");
        }
    }
    
    if( mEnableOculus )
    {
        // Setup Extra Window
        if( Display::getDisplays().size() > 1 )
        {
            WindowRef secondWindow = createWindow();
            secondWindow->setSize( 1280, 800 );
        }
        else
        {
            setWindowSize( 1280, 800 );
        }
        
        mOculusCam.setup();
        mDistortionHelper = ovr::DistortionHelper::create();
    }
    
    mLastElapsedSeconds = getElapsedSeconds();
    mElapsedSecondsThisFrame = 0.0f;
    
    mInfoPanel.init();
    
    console() << "[main] vsync: " << (gl::isVerticalSyncEnabled() ? "on" : "off") << std::endl;
    
    setupScenes();
}

void OculonApp::shutdown()
{
    console() << "[main] shutting down...\n";
    
    if( mIsCapturingVideo )
    {
        stopVideoCapture();
    }
    
    for (tSceneList::iterator sceneIt = mScenes.begin();
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
    
    if( mEnableMindWave )
    {
        mMindWave.shutdown();
    }
}

void OculonApp::setupInterface()
{
    mThumbnailControls.clear();
    mInterface = new Interface(this, &mOscServer);
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("OCULON");
    // main
    mInterface->addButton(CreateTriggerParam("sync", NULL)
                          .oscReceiver("master", "sync"))->registerCallback( this, &OculonApp::syncInterface );
    
    
    mInterface->addParam(CreateBoolParam("mouse", &mEnableManualCamControls));
    // output mode
    mInterface->gui()->addLabel("--render--");
    vector<string> outputModeNames;
#define OUTPUTMODE_ENTRY( nam, enm ) \
outputModeNames.push_back(nam);
    OUTPUTMODE_TUPLE
#undef  OUTPUTMODE_ENTRY
    mInterface->addEnum(CreateEnumParam("Output Mode", (int*)&mOutputMode)
                        .maxValue(OUTPUT_COUNT), outputModeNames)->registerCallback( this, &OculonApp::onOutputModeChange );
    if (mOutputMode == OUTPUT_FBO)
    {
        mInterface->addParam(CreateIntParam("Visible Layer", &mVisibleLayerIndex)
                             .maxValue(MAX_LAYERS-1));
    }
    mInterface->addParam(CreateBoolParam("Syphon", &mEnableSyphonServer)
                         .oscReceiver("master", "syphon"));
    mInterface->addParam(CreateFloatParam("BG Alpha", &mBackgroundAlpha));
    mInterface->addParam(CreateBoolParam("Draw FBOs", &mDrawToScreen));
    
    // capture
    mInterface->gui()->addSeparator();
    mInterface->gui()->addLabel("--capture--");
    mInterface->addButton(CreateTriggerParam("Screenshot", NULL))->registerCallback(this, &OculonApp::triggerScreenshot);
    mInterface->addParam(CreateBoolParam("Capture Debug", &mCaptureDebugOutput));
    mInterface->addParam(CreateBoolParam("Capture Frames", &mIsCapturingFrames))->registerCallback( this, &OculonApp::onFrameCaptureToggle );
    mInterface->addParam(CreateFloatParam("Capture Duration", &mCaptureDuration)
                         .minValue(1.0f)
                         .maxValue(600.0f));
    // audio
    mInterface->gui()->addSeparator();
    mInterface->gui()->addLabel("--audio--");
    mAudioInput.setupInterface( mInterface );
}

bool OculonApp::syncInterface()
{
    mInterface->sendAll();
    for (tSceneList::iterator sceneIt = mScenes.begin();
         sceneIt != mScenes.end();
         ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        scene->mInterface->sendAll();
    }
    return false;
}

bool OculonApp::toggleScene(const int sceneId)
{
    if( sceneId >= 0 && sceneId < mScenes.size() )
    {
        mScenes[sceneId]->toggleActiveVisible();
        if( mScenes[sceneId]->isVisible() )
        {
            mLastActiveScene = sceneId;
            mVisibleLayerIndex = mScenes[sceneId]->getLayerIndex();
        }
    }
    
    return false;
}

bool OculonApp::showInterface(const int sceneId)
{
    if( sceneId < 0 )
    {
        // hide all scene interfaces
        for (tSceneList::iterator sceneIt = mScenes.begin();
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
        // also enables the scene
        mScenes[sceneId]->setVisible(true);
        mScenes[sceneId]->setRunning(true);
        mScenes[sceneId]->showInterface(true);
        mInterface->gui()->setEnabled(false);
        mLastActiveScene = sceneId;
        mVisibleLayerIndex = mScenes[sceneId]->getLayerIndex();
    }
    
    return true;
}

//MARK: Setup Scenes
void OculonApp::setupScenes()
{
    console() << "[main] creating scenes...\n";
    
    mScenes.clear();
    
    if( mConfig.getBool("audio") )      addScene( new AudioSignal() );
    if( mConfig.getBool("orbiter") )    addScene( new Orbiter() );
    if( mConfig.getBool("catalog") )    addScene( new Catalog() );
    if( mConfig.getBool("parsec") )     addScene( new Parsec() );
    if( mConfig.getBool("binned") )     addScene( new Binned() );
    if( mConfig.getBool("graviton") )   addScene( new Graviton() );
    if( mConfig.getBool("dust") )       addScene( new Dust() );
    
    if( mConfig.getBool("tectonic") )   addScene( new Tectonic() );
    if( mConfig.getBool("flock") )      addScene( new Flock() );
    if( mConfig.getBool("trails") )     addScene( new Trails() );
    if( mConfig.getBool("polyhedron") ) addScene( new Polyhedron() );
    if( mConfig.getBool("terrain") )    addScene( new Deformer() );
    if( mConfig.getBool("lines") )      addScene( new Lines() );
    if( mConfig.getBool("contour") )    addScene( new Contour() );
    if( mConfig.getBool("grid") )       addScene( new Grid() );
    if( mConfig.getBool("textorbit") )  addScene( new TextOrbit() );
    
    if( mConfig.getBool("corona") )     addScene( new Corona() );
    if( mConfig.getBool("fluid") )      addScene( new Fluid() );
    
    // Shader Scenes
    if( mConfig.getBool("cells") )              addScene( new Cells() );
    if( mConfig.getBool("circlewave") )         addScene( new CircleWave() );
    if( mConfig.getBool("rings") )              addScene( new Rings() );
    if( mConfig.getBool("tilings") )            addScene( new Tilings() );
    if( mConfig.getBool("voronoi") )            addScene( new Voronoi() );
    if( mConfig.getBool("oscillator") )         addScene( new Oscillator() );
    if( mConfig.getBool("objshaders") )         addScene( new ObjectShaders() );
    if( mConfig.getBool("textureshaders") )     addScene( new TextureShaders("textureshaders") );
    
    // Test Scenes
    //addScene( new MovieTest() );
    if( mConfig.getBool("shadertest") ) addScene( new ShaderTest() );
    if( mConfig.getBool("fisheye_test") ) addScene( new FisheyeTest() );
    //if( mConfig.getBool("kinect_test") ) addScene( new SkeletonTest() );
    //if( mEnableKinect && mConfig.getBool("kinect_test") ) addScene( new KinectTest() );
    if( mEnableMindWave && mConfig.getBool("mindwave_test") ) addScene( new MindWaveTest() );
}

void OculonApp::addScene(Scene* scene, bool autoStart)
{
    console() << (mScenes.size()+1) << ": " << scene->getName() << std::endl;
    
    scene->init(this);
    if( mSetupScenesOnStart )
    {
        scene->setup();
        scene->setupInterfaces();
    }
    
    if( autoStart )
    {
        scene->setRunning(true);
        scene->setVisible(true);
        scene->setDebug(true);
        mLastActiveScene = mScenes.size();
    }
    mScenes.push_back(scene);
    mSceneMap[scene->getName()] = scene;
    const int sceneIndex = mScenes.size()-1;
    
    // interface
    if( (sceneIndex) % 2 == 0 )
    {
        mInterface->gui()->addColumn();
    }
    else
    {
        mInterface->gui()->addSeparator();
    }
    std::stringstream labelss;
    if (sceneIndex < 9)
    {
        labelss << scene->getName() << " (" << sceneIndex+1 << ")";
    }
    else
    {
        labelss << scene->getName();
    }
    mInterface->gui()->addButton(labelss.str())->registerCallback( boost::bind( &OculonApp::showInterface, this, sceneIndex) );
    // scene thumbnails
    if( scene->getFbo() )
    {
        mowa::sgui::TextureVarControl* texControl = mInterface->gui()->addParam(scene->getName(), &(scene->getFbo().getTexture()));
        texControl->registerCallback( boost::bind( &OculonApp::showInterface, this, sceneIndex) );
        mThumbnailControls.push_back( texControl );
    }
    //mInterface->gui()->addButton("toggle")->registerCallback( boost::bind( &OculonApp::toggleScene, this, mScenes.size()-1) );
    mInterface->addParam(CreateBoolParam("on", &(scene->mIsVisible)))->registerCallback( scene, &Scene::setRunningByVisibleState );
    mInterface->addParam(CreateBoolParam("debug", &(scene->mIsDebug)))->registerCallback( scene, &Scene::onDebugChanged );
}

Scene* OculonApp::getScene(const std::string& name)
{
    tSceneMap::iterator it = mSceneMap.find(name);
    if( it != mSceneMap.end() )
    {
        return it->second;
    }
    else
    {
        return NULL;
    }
}

void OculonApp::resize()
{
    CameraPersp cam = mMayaCam.getCamera();
    cam.setAspectRatio( getWindowAspectRatio() );
    mMayaCam.setCurrentCam( cam );
    
    int index = 0;
    for (tSceneList::iterator sceneIt = mScenes.begin();
         sceneIt != mScenes.end();
         ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        if( scene && scene->isRunning() )
        {
            scene->resize();
            if( scene->getFbo() )
            {
                //mThumbnailControls[index]->resetTexture( &(scene->getFbo().getTexture()) );
            }
        }
        ++index;
    }
}

void OculonApp::mouseMove( MouseEvent event )
{
    for (tSceneList::iterator sceneIt = mScenes.begin();
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
    if (mEnableManualCamControls)
    {
        // let the camera handle the interaction
        mMayaCam.mouseDown( event.getPos() );
    }
    
    for (tSceneList::iterator sceneIt = mScenes.begin();
         sceneIt != mScenes.end();
         ++sceneIt )
    {
        Scene* scene = (*sceneIt);
        if( scene && scene->isRunning() )
        {
            scene->handleMouseDown(event);
        }
    }
    
    console() << "[mouse] click (" << event.getPos().x << "," << event.getPos().y << ")" << std::endl;
}

void OculonApp::mouseDrag( MouseEvent event )
{
    if (mEnableManualCamControls)
    {
        // let the camera handle the interaction
        mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
    }
    
    
    for (tSceneList::iterator sceneIt = mScenes.begin();
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
    for (tSceneList::iterator sceneIt = mScenes.begin();
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
                mParams->show( !mParams->isVisible() );
            }
            else
            {
                for (tSceneList::iterator sceneIt = mScenes.begin();
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
                    bool useDefaultSettings = event.isAltDown() ? false : true;
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
        for (tSceneList::iterator sceneIt = mScenes.begin();
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
        mInfoPanel.addLine( buf, Color(0.4f, 0.5f, 1.0f) );
        snprintf(buf, BUFSIZE, "%d x %d", getWindowWidth(), getWindowHeight());
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
        }
        
        if( mIsCapturingVideo || mIsCapturingFrames )
        {
            snprintf(buf, BUFSIZE, "-- duration: %.1fs / %.1fs", mFrameCaptureCount/kCaptureFramerate, mCaptureDuration);
            mInfoPanel.addLine( buf, Color(0.9f,0.5f,0.5f) );
            
            float time = (mCaptureDuration*kCaptureFramerate - mFrameCaptureCount) * mElapsedSecondsThisFrame;
            if( time > 60.f )
            {
                const int min = time / 60;
                snprintf(buf, BUFSIZE, "-- finish in %dm%f.0s", min, fmod(time,60.0f));
            }
            else
            {
                snprintf(buf, BUFSIZE, "-- finish in %.0fs", time);
            }
            mInfoPanel.addLine( buf, Color(0.9f,0.5f,0.5f) );
        }
    }
    
    // TODO: clean up this mess!
    if( mIsCapturingFrames )
    {
        const float elapsed = (mFrameCaptureCount/kCaptureFramerate);
        if( elapsed >= mCaptureDuration )
        {
            enableFrameCapture(false);
        }
    }
    
    if( mIsCapturingVideo )
    {
        const float elapsed = (mFrameCaptureCount/kCaptureFramerate);
        if( elapsed >= mCaptureDuration )
        {
            stopVideoCapture();
        }
    }

    const float dt = (mIsCapturingFrames || mIsCapturingVideo) ? (1.0f/kCaptureFramerate) : mElapsedSecondsThisFrame;
    
    mAudioInput.update();
    mAudioInputHandler.update( dt, mAudioInput );
    
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
//        mKinectController.update(); // TODO: update kinect setup
    }
    
    if( mEnableOculus )
    {
        mOculusCam.update();
    }
    
    // update scenes
    for (tSceneList::iterator sceneIt = mScenes.begin();
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
                    scene->setupInterfaces();
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
    if( mDrawToScreen )
    {
        drawScenes();
    }
}

void OculonApp::drawToFbo( gl::Fbo& fbo )
{
    Area prevViewport = gl::getViewport();
    // bind the framebuffer - now everything we draw will go there
    fbo.bindFramebuffer();
    
    gl::disableDepthRead();
    gl::disableDepthWrite();
    gl::disableAlphaBlending();
    
    // setup the viewport to match the dimensions of the FBO
    gl::setViewport( fbo.getBounds() );
    gl::clear( ColorA(0.0f,0.0f,0.0f,mBackgroundAlpha) );
    
    drawScenes(0);
    
    fbo.unbindFramebuffer();
    
    gl::setViewport( prevViewport );
}

void OculonApp::drawToLayers()
{
    for( int layerIndex = 0; layerIndex < MAX_LAYERS; ++layerIndex )
    {
        gl::Fbo& fbo = mFbo[layerIndex];
        
        Area prevViewport = gl::getViewport();
        // bind the framebuffer - now everything we draw will go there
        fbo.bindFramebuffer();
        
        gl::disableDepthRead();
        gl::disableDepthWrite();
        gl::disableAlphaBlending();
        
        // setup the viewport to match the dimensions of the FBO
        gl::setViewport( fbo.getBounds() );
        gl::clear( ColorA(0.0f,0.0f,0.0f,mBackgroundAlpha) );
        
        drawScenes(layerIndex);
        
        fbo.unbindFramebuffer();
        
        gl::setViewport( prevViewport );
    }
}

void OculonApp::drawFromFbo( gl::Fbo& fbo )
{
    if (!mDrawToScreen)
    {
        return;
    }
    
    // draw the captured texture back to screen
    gl::color( ColorA(1.0f,1.0f,1.0f,1.0f) );
    gl::setMatricesWindow( getWindowSize() );
    
    float width = math<float>::min(getWindowWidth(), fbo.getWidth());
    float height = math<float>::min(getWindowHeight(), fbo.getHeight());
    
    // maintain aspect ratio
    float ratio = fbo.getHeight() / fbo.getWidth();
    height = math<float>::min(height, getWindowWidth() * ratio);
    
    // flip
    gl::draw( fbo.getTexture(), Rectf( 0, getWindowHeight(), width, getWindowHeight() - height ) );
}

void OculonApp::renderScenes()
{
    // render scenes to FBO
    for (tSceneList::iterator sceneIt = mScenes.begin();
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

void OculonApp::drawScenes(int layerIndex)
{
    if( mOutputMode == OUTPUT_MULTIFBO )
    {
        // draw scene FBO textures
        gl::disableDepthRead();
        gl::disableDepthWrite();
        gl::enableAlphaBlending();
        glEnable(GL_TEXTURE_2D);
        gl::color( ColorA(1.0f,1.0f,1.0f,1.0f) );
        gl::setViewport( getWindowBounds() );
        
        float width = math<float>::min(getWindowWidth(), getViewportWidth());
        float height = math<float>::min(getWindowHeight(), getViewportHeight());
        // maintain aspect ratio
        //float ratio = fbo.getHeight() / fbo.getWidth();
        height = math<float>::min(height, getWindowWidth() / getViewportAspectRatio());
        float y1 = getWindowHeight() - height;
        float y2 = y1 + height;
        
        if( mEnableOculus )
        {
            if( mLastActiveScene >= 0 && mLastActiveScene < mScenes.size() )
            {
                Scene* scene = mScenes[mLastActiveScene];
                if( scene && scene->isVisible() )
                {
                    mDistortionHelper->render( scene->getFboTexture(), getWindowBounds() );
//                    gl::draw( scene->getFboTexture(), Rectf( 0, y1, width, y2 ) );
                }
            }
        }
        else if( mDrawOnlyLastScene )
        {
            if( mLastActiveScene >= 0 && mLastActiveScene < mScenes.size() )
            {
                Scene* scene = mScenes[mLastActiveScene];
                if( scene && scene->isVisible() )
                {
                    gl::draw( scene->getFboTexture(), Rectf( 0, y1, width, y2 ) );
                }
            }
        }
        else
        {
            for (tSceneList::iterator sceneIt = mScenes.begin();
                 sceneIt != mScenes.end();
                 ++sceneIt )
            {
                Scene* scene = (*sceneIt);
                assert( scene != NULL );
                if( scene && scene->isVisible() )
                {
                    gl::Fbo& fbo = scene->getFbo();
                    gl::draw( fbo.getTexture(), Rectf( 0, 0, fbo.getWidth(), fbo.getHeight() ) );
                }
            }
        }
    }
    else
    {
        // draw scenes directly
        for (tSceneList::iterator sceneIt = mScenes.begin();
             sceneIt != mScenes.end();
             ++sceneIt )
        {
            Scene* scene = (*sceneIt);
            assert( scene != NULL );
            if( scene && scene->isVisible() && scene->getLayerIndex() == layerIndex )
            {
                scene->draw();
            }
        }
    }
}

void OculonApp::draw()
{
    gl::clear();
    
    if( mDebugRender )
    {
        mScenes[0]->draw();
        return;
    }
    
    
    switch( mOutputMode )
    {
        case OUTPUT_FBO:
            //drawToFbo(mFbo[mVisibleLayerIndex]);
            drawToLayers();
            break;
            
        case OUTPUT_MULTIFBO:
            renderScenes();
            break;
            
        case OUTPUT_DOME:
            drawToFbo(mFbo[mVisibleLayerIndex]);
            break;
            
        default:
            drawToScreen();
            break;
    }
    
    switch( mOutputMode )
    {
        case OUTPUT_FBO:
            drawFromFbo(mFbo[mVisibleLayerIndex]);
            break;
            
        case OUTPUT_DOME:
            mDomeRenderer.renderFboToDome( mFbo[mVisibleLayerIndex], getMayaCam() );
            drawFromFbo( mDomeRenderer.getDomeProjectionFbo() );
            break;
            
        case OUTPUT_MULTIFBO:
            drawToScreen();
            break;
            
        default:
            break;
    }
    
    if( mCaptureDebugOutput )
    {
        drawDebug();
        // HACKHACK: to show interface through Resolume
        if( mEnableSyphonServer )
        {
            mScreenSyphon[0].publishScreen();
        }
    }
    
    // capture video
	if( mIsCapturingVideo )
    {
        ++mFrameCaptureCount;
        if( mOutputMode == OUTPUT_MULTIFBO && mLastActiveScene >= 0 && mLastActiveScene < mScenes.size() )
        {
            gl::Texture& tex = mScenes[mLastActiveScene]->getFbo().getTexture();
            tex.setFlipped();
            Surface surface = Surface(tex);
            mMovieWriter.addFrame( surface );
            // not sure why but copyWindowSurface is faster...
            //mMovieWriter.addFrame( copyWindowSurface() );
        }
        else
        {
            mMovieWriter.addFrame( copyWindowSurface() );
        }
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
    
    if( mEnableSyphonServer )
    {
        const bool flipTexture = false;
        switch( mOutputMode )
        {
            case OUTPUT_DIRECT:
                mScreenSyphon[mVisibleLayerIndex].publishScreen();
                break;
                
            case OUTPUT_FBO:
            {
                for( int layerIndex = 0; layerIndex < MAX_LAYERS; ++layerIndex )
                {
                    mScreenSyphon[layerIndex].publishTexture(&mFbo[layerIndex].getTexture(), flipTexture);
                }
            }
                break;
                
            case OUTPUT_MULTIFBO:
                // publish each scene separately
                for (tSceneList::iterator sceneIt = mScenes.begin();
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
                
            case OUTPUT_DOME:
                mScreenSyphon[mVisibleLayerIndex].publishTexture( & mDomeRenderer.getDomeProjectionFbo().getTexture(), flipTexture );
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
    gl::setViewport( getWindowBounds() );
    gl::pushMatrices();
    gl::setMatricesWindow( Vec2i( getWindowWidth(), getWindowHeight() ) );
    for (tSceneList::iterator sceneIt = mScenes.begin();
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
    
    gl::pushMatrices();
    gl::setMatricesWindow( Vec2i( getWindowWidth(), getWindowHeight() ) );
    if( mInfoPanel.isVisible() )
    {
        mInfoPanel.render( Vec2f( getWindowWidth(), getWindowHeight() ) );
    }
    
    if( !mIsPresentationMode )
    {
        mInterface->draw();
        mParams->draw();
    }
    gl::popMatrices();
}

#pragma MARK Capture

void OculonApp::captureFrames()
{
    ++mFrameCaptureCount;
    
    if( mOutputMode == OUTPUT_FBO )
    {
        writeImage( mFrameCapturePath + "/" + Utils::leftPaddedString( toString(mFrameCaptureCount) ) + ".png", mFbo[mVisibleLayerIndex].getTexture() );
    }
    else if( mOutputMode == OUTPUT_MULTIFBO && mLastActiveScene >= 0 && mLastActiveScene < mScenes.size() )
    {
        writeImage( mFrameCapturePath + "/" + Utils::leftPaddedString( toString(mFrameCaptureCount) ) + ".png", mScenes[mLastActiveScene]->getFboTexture() );
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
        writeImage( p, mFbo[mVisibleLayerIndex].getTexture() );
    }
    else if( mOutputMode == OUTPUT_MULTIFBO && mLastActiveScene >= 0 && mLastActiveScene < mScenes.size() )
    {
        writeImage( p, mScenes[mLastActiveScene]->getFboTexture() );
    }
    else
    {
        writeImage( p, copyWindowSurface() );
    }
}

bool OculonApp::triggerScreenshot()
{
    mSaveNextFrame = true;
    return false;
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
    mFrameCaptureCount = 0;
    
    // spawn file dialog
    // outputPath = getSaveFilePath();
    
    if( useDefaultPath )
    {
        // H.264, 30fps, high detail = 1635148593
        // DXV, 30/60fps, alpha = 1146635337
        mwFormat.setCodec(1146635337); // get this value from the output of the dialog
        mwFormat.setTimeScale(3000); // 30 fps
        mwFormat.setDefaultDuration(1.0f/30.0f);
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
        mMovieWriter.setWriter(qtime::MovieWriter( outputPath, getViewportWidth(), getViewportHeight(), mwFormat ));
        mMovieWriter.start();
        mIsCapturingVideo = true;
    }
}

void OculonApp::stopVideoCapture()
{
    console() << "[main] stop video capture\n";
    mMovieWriter.finish();
    mMovieWriter.stop();
    mIsCapturingVideo = false;
    mFrameCaptureCount = 0;
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
        fs::path outputPath;
        if( mOutputMode == OUTPUT_MULTIFBO  && mLastActiveScene >= 0 && mLastActiveScene < mScenes.size() )
        {
            outputPath = Utils::getUniquePath("~/Documents/oculon/capture/" + mScenes[mLastActiveScene]->getName());
        }
        else
        {
            outputPath = Utils::getUniquePath("~/Documents/oculon/capture/oculon_capture");
        }
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
        resize();
    }
    else
    {
        resize();
    }
    
    return false;
}

int OculonApp::getViewportWidth() const
{
    if( mOutputMode == OUTPUT_FBO )
    {
        return mFbo[mVisibleLayerIndex].getWidth();
    }
    else
    {
        return mViewportWidth;
    }
}

int OculonApp::getViewportHeight() const
{
    if( mOutputMode == OUTPUT_FBO )
    {
        return mFbo[mVisibleLayerIndex].getHeight();
    }
    else
    {
        return mViewportHeight;
    }
}

Area OculonApp::getViewportBounds() const
{
    if( mOutputMode == OUTPUT_FBO )
    {
        return Area( 0, 0, mFbo[mVisibleLayerIndex].getWidth(), mFbo[mVisibleLayerIndex].getHeight() );
    }
    else
    {
        return Area( 0, 0, mViewportWidth, mViewportHeight );
    }
}

Vec2i OculonApp::getViewportSize() const
{
    if( mOutputMode == OUTPUT_FBO )
    {
        return Vec2i( mFbo[mVisibleLayerIndex].getWidth(), mFbo[mVisibleLayerIndex].getHeight() );
    }
    else
    {
        return Vec2i( mViewportWidth, mViewportHeight );
    }
}

float OculonApp::getViewportAspectRatio() const
{
    if( mOutputMode == OUTPUT_FBO )
    {
        return (mFbo[mVisibleLayerIndex].getWidth() / (float)mFbo[mVisibleLayerIndex].getHeight());
    }
    else
    {
        return ((float)mViewportWidth / (float)mViewportHeight);
    }
}

void OculonApp::setCamera(const Vec3f &eye, const Vec3f &look, const Vec3f &up)
{
    CameraPersp cam;
    //cam.lookAt( Vec3f( 0.0f, 0.0f, 750.0f ), Vec3f::zero(), Vec3f(0.0f, 1.0f, 0.0f) );
    cam.setEyePoint( eye );
	cam.setCenterOfInterestPoint( look );
	cam.setPerspective( 45.0f, getWindowAspectRatio(), 1.0f, 200000.0f );
    mMayaCam.setCurrentCam( cam );
}

CINDER_APP_NATIVE( OculonApp, RendererGl(0) )
