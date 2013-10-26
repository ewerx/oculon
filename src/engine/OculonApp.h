/*
 *  OculonApp.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-16.
 *  Copyright 2013 ewerx. All rights reserved.
 *
 */

#ifndef __OCULONAPP_H__
#define __OCULONAPP_H__

#include "AudioInput.h"
#include "CameraController.h"
#include "DomeRenderer.h"
#include "InfoPanel.h"
//#include "KinectController.h" // TODO: Kinect removed due to errors
#include "MidiInput.h"
#include "MindWave.h"
#include "OscServer.h"
#include "SimpleGUI.h"

#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/app/AppNative.h"
#include "cinder/audio/Input.h"
#include "cinder/gl/Fbo.h"
#include "cinder/params/Params.h"
//#include "cinder/qtime/MovieWriter.h" // TODO: removed due to WindowRef compile error: https://forum.libcinder.org/topic/porting-windows-app-to-osx
#include "cinderSyphon.h"

#include <vector>
#include <boost/unordered_map.hpp>


// fwd decl
class Scene;
class Interface;

// main app
//
class OculonApp : public AppBasic 
{
public:
    //TODO: cleanup
    enum eScenes
    {
        SCENE_ORBITER,
        SCENE_BINNED
    };
    
    enum eSpecialSceneIndex
    {
        INTERFACE_MAIN = -1,
        INTERFACE_NONE = -2
    };
    
    enum { MAX_LAYERS = 1 };

#define OUTPUTMODE_TUPLE \
OUTPUTMODE_ENTRY( "Direct", OUTPUT_DIRECT ) \
OUTPUTMODE_ENTRY( "FBO", OUTPUT_FBO ) \
OUTPUTMODE_ENTRY( "Multi-FBO", OUTPUT_MULTIFBO ) \
OUTPUTMODE_ENTRY( "Dome", OUTPUT_DOME ) \
//end tuple
    enum eOutputMode
    {
#define OUTPUTMODE_ENTRY( nam, enm ) \
enm,
        OUTPUTMODE_TUPLE
#undef  OUTPUTMODE_ENTRY
        
        OUTPUT_COUNT
    };
    
public: // cinder interface
    void prepareSettings( Settings *settings );
	void setup();
	void update();
    void resize();
    void shutdown();
    
    void mouseMove( ci::app::MouseEvent event );
	void mouseDrag( ci::app::MouseEvent event );
    void mouseDown( ci::app::MouseEvent event );	
    void mouseUp( ci::app::MouseEvent event );
    void keyDown( ci::app::KeyEvent event );

	void draw();
    
    int         getViewportWidth() const;                 
    int         getViewportHeight() const;
    ci::Area    getViewportBounds() const;
    ci::Vec2i   getViewportSize() const;
    float       getViewportAspectRatio() const;
    
public: // new
    AudioInput& getAudioInput()                         { return mAudioInput; }
    MidiInput& getMidiInput()                           { return mMidiInput; }
    MindWave& getMindWave()                             { return mMindWave; }
    //KinectController& getKinectController()             { return mKinectController; }
    OscServer& getOscServer()                           { return mOscServer; }
                
    InfoPanel& getInfoPanel()                           { return mInfoPanel; }
    ci::params::InterfaceGlRef getParams()              { return mParams; }
    
    inline bool isPresentationMode() const              { return mIsPresentationMode; }
    inline void setUseMayaCam(bool use)                 { mUseMayaCam = use; }
    
    void setCamera( const ci::Vec3f& eye, const ci::Vec3f& look, const ci::Vec3f& up );
    const ci::Camera& getMayaCam() const                    { return mMayaCam.getCamera(); }
    //const Camera& getGlobalCam()                        { return mCameraController.getCamera(); }
    
    inline double getElapsedSecondsThisFrame() const    { return mElapsedSecondsThisFrame; }
    
    float getBackgroundAlpha() const                    { return mBackgroundAlpha; }
    
    //TODO: hack
    Scene* getScene(const int index)                    { return ( index < mScenes.size() ) ? mScenes[index] : NULL ; }
    Scene* getScene(const std::string& name);
    
    void enableFrameCapture( const bool enable );
    void setCaptureDuration( const float duration ) { mCaptureDuration = duration; }
    int getFrameCaptureCount() const { return mFrameCaptureCount; }
    
    // interface callbacks
    bool onFrameCaptureToggle();
    bool onOutputModeChange();
    bool syncInterface();
    
    bool toggleScene(const int sceneId);
    bool showInterface(const int sceneId);
    
    const Config& getConfig() const { return mConfig; }
    eOutputMode getOutputMode() const { return mOutputMode; }
    
protected: // new
    
    void drawToScreen();
    void drawToFbo(ci::gl::Fbo& fbo);
    void drawToLayers();
    void drawFromFbo( ci::gl::Fbo& fbo );
    void drawScenes(int layerIndex =0);
    void renderScenes();
    void drawDebug();
    void captureFrames();
    void saveScreenshot();
    bool triggerScreenshot();
    
    void setupInterface();
    void setupScenes();
    void addScene(Scene* newScene, bool startActive =false);
    void setPresentationMode( bool enabled );
    void toggleFullscreen();
    
    //TODO: quicktime removed due to compile errors
//    void startVideoCapture( bool useDefaultPath =true );
//    void stopVideoCapture();
    
    
private: // members
    
    // settings
    Config                  mConfig;
    
    // input
    AudioInput              mAudioInput;
    
    MidiInput               mMidiInput;
    bool                    mEnableMidi;
    
    MindWave                mMindWave;
    bool                    mEnableMindWave;
    
    OscServer               mOscServer;
    bool                    mEnableOscServer;
    
//    KinectController        mKinectController; // TODO: kinect
    bool                    mEnableKinect;
    bool                    mEnableOpenNI;
    
    // render
    double                  mLastElapsedSeconds;
    double                  mElapsedSecondsThisFrame;
    ci::MayaCamUI           mMayaCam;
    bool                    mUseMayaCam;
    bool                    mDebugRender;
    //CameraController        mCameraController;
    
    
    eOutputMode             mOutputMode;
    ci::gl::Fbo             mFbo[MAX_LAYERS];
    int                     mVisibleLayerIndex;
    float                   mBackgroundAlpha;
    int                     mViewportWidth;
    int                     mViewportHeight;
    
    DomeRenderer            mDomeRenderer;
    
    // scenes
    typedef boost::unordered_map<std::string, Scene*> tSceneMap;
    typedef std::vector<Scene*>  tSceneList;
    
    tSceneList              mScenes;
    tSceneMap               mSceneMap;
    bool                    mDrawToScreen;
    bool                    mDrawOnlyLastScene;
    int                     mLastActiveScene;
    bool                    mSetupScenesOnStart;
    
    // ui
    InfoPanel                   mInfoPanel;
    ci::params::InterfaceGlRef	mParams;
    Interface*                  mInterface;
    bool                        mIsPresentationMode;
    std::vector<mowa::sgui::TextureVarControl*> mThumbnailControls;
    
    // capture
    bool                    mIsCapturingVideo;
    //ci::qtime::MovieWriter  mMovieWriter; // TODO: removed due to WindowRef compile error: https://forum.libcinder.org/topic/porting-windows-app-to-osx
    
    bool                    mIsCapturingFrames;
    std::string             mFrameCapturePath;
    int                     mFrameCaptureCount;
    float                   mCaptureDuration;
    bool                    mSaveNextFrame;
    bool                    mCaptureDebugOutput;
    
    // syphon
    syphonServer            mScreenSyphon[MAX_LAYERS];
    //syphonClient            mSyphonClient;
    bool                    mEnableSyphonServer;
    
    // fps
    bool                    mIsSendingFps;
    float                   mFpsSendTimer;
};

#endif