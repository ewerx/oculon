/*
 *  Oculon.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-16.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __OCULONAPP_H__
#define __OCULONAPP_H__

#include "AudioInput.h"
#include "DomeRenderer.h"
#include "InfoPanel.h"
#include "KinectController.h"
#include "MidiInput.h"
#include "MindWave.h"
#include "OscServer.h"
#include "SimpleGUI.h"

#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/app/AppBasic.h"
#include "cinder/audio/Input.h"
#include "cinder/gl/Fbo.h"
#include "cinder/params/Params.h"
#include "cinder/qtime/MovieWriter.h"
#include "cinderSyphon.h"

#include <vector>
#include <boost/unordered_map.hpp>

// fwd decl
class Scene;
class Interface;

using namespace ci;
using namespace ci::app;
using std::vector;

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
    void resize( ResizeEvent event );
    void shutdown();
    
    void mouseMove( MouseEvent event );
	void mouseDrag( MouseEvent event );
    void mouseDown( MouseEvent event );	
    void mouseUp( MouseEvent event );
    void keyDown( KeyEvent event );

	void draw();
    
    int     getViewportWidth() const;                 
    int     getViewportHeight() const;
    Area    getViewportBounds() const;
    Vec2i   getViewportSize() const;
    float   getViewportAspectRatio() const;
    
public: // new
    inline AudioInput& getAudioInput()                  { return mAudioInput; }
    inline MidiInput& getMidiInput()                    { return mMidiInput; }
    inline MindWave& getMindWave()                      { return mMindWave; }
    inline KinectController& getKinectController()      { return mKinectController; }
    inline OscServer& getOscServer()                    { return mOscServer; }
    
    inline InfoPanel& getInfoPanel()                    { return mInfoPanel; }
    inline params::InterfaceGl& getParams()             { return mParams; }
    
    inline bool isPresentationMode() const              { return mIsPresentationMode; }
    inline void setUseMayaCam(bool use)                 { mUseMayaCam = use; }
    
    void setCamera( const Vec3f& eye, const Vec3f& look, const Vec3f& up );
    inline const Camera& getMayaCam() const             { return mMayaCam.getCamera();  }
    
    inline double getElapsedSecondsThisFrame() const    { return mElapsedSecondsThisFrame; }
    
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
    void drawToFbo( gl::Fbo& fbo );
    void drawFromFbo( gl::Fbo& fbo );
    void drawScenes();
    void renderScenes();
    void drawDebug();
    void captureFrames();
    void saveScreenshot();
    
    void setupInterface();
    void setupScenes();
    void addScene(Scene* newScene, bool startActive =false);
    void setPresentationMode( bool enabled );
    void toggleFullscreen();
    
    void startVideoCapture( bool useDefaultPath =true );
    void stopVideoCapture();
    
    
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
    
    KinectController        mKinectController;
    bool                    mEnableKinect;
    bool                    mEnableOpenNI;
    
    // render
    double                  mLastElapsedSeconds;
    double                  mElapsedSecondsThisFrame;
    MayaCamUI               mMayaCam;
    bool                    mUseMayaCam;
    bool                    mDebugRender;
    
    eOutputMode             mOutputMode;
    gl::Fbo                 mFbo;
    
    DomeRenderer            mDomeRenderer;
    
    // scenes
    typedef boost::unordered_map<std::string, Scene*> tSceneMap;
    typedef vector<Scene*>  tSceneList;
    
    tSceneList              mScenes;
    tSceneMap               mSceneMap;
    bool                    mDrawToScreen;
    bool                    mDrawOnlyLastScene;
    int                     mLastActiveScene;
    bool                    mSetupScenesOnStart;
    
    // ui
    InfoPanel               mInfoPanel;
    params::InterfaceGl		mParams;
    Interface*              mInterface;
    bool                    mIsPresentationMode;
    std::vector<mowa::sgui::TextureVarControl*> mThumbnailControls;
    
    // capture
    bool                    mIsCapturingVideo;
    qtime::MovieWriter      mMovieWriter;
    
    bool                    mIsCapturingFrames;
    string                  mFrameCapturePath;
    int                     mFrameCaptureCount;
    float                   mCaptureDuration;
    bool                    mSaveNextFrame;
    bool                    mCaptureDebugOutput;
    
    // syphon
    syphonServer            mScreenSyphon;
    bool                    mEnableSyphonServer;
    
    // fps
    bool                    mIsSendingFps;
    float                   mFpsSendTimer;
};

#endif