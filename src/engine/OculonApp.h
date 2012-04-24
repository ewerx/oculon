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

#include <vector>
#include "cinder/app/AppBasic.h"
#include "cinder/audio/Input.h"
#include "cinder/Camera.h"
#include "cinder/params/Params.h"
#include "cinder/MayaCamUI.h"
#include "cinder/qtime/MovieWriter.h"
#include "cinder/gl/Fbo.h"
#include "cinderSyphon.h"
#include "AudioInput.h"
#include "MidiInput.h"
#include "MindWave.h"
#include "OscServer.h"
#include "KinectController.h"
#include "InfoPanel.h"

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
    
    int getViewportWidth() const;                 
    int getViewportHeight() const;
    
public: // new
    inline AudioInput& getAudioInput()               { return mAudioInput; }
    inline MidiInput& getMidiInput()                 { return mMidiInput; }
    inline MindWave& getMindWave()                   { return mMindWave; }
    inline KinectController& getKinectController()   { return mKinectController; }
    inline OscServer& getOscServer()                 { return mOscServer; }
    
    inline const Vec2f& getMousePos()          { return mMousePos; }
    inline InfoPanel& getInfoPanel()           { return mInfoPanel; }
    inline params::InterfaceGl& getParams()    { return mParams; }
    
    inline bool isPresentationMode() const     { return mIsPresentationMode; }
    inline void setUseMayaCam(bool use)        { mUseMayaCam = use; }
    
    void setCamera( const Vec3f& eye, const Vec3f& look, const Vec3f& up );
    inline const Camera& getMayaCam() const     { return mMayaCam.getCamera();  }
    
    inline double getElapsedSecondsThisFrame() const  { return mElapsedSecondsThisFrame; }
    
    //TODO: hack
    Scene* getScene(const int index)            { return ( index < mScenes.size() ) ? mScenes[index] : NULL ; }
    
    void enableFrameCapture( const bool enable );
    
    // interface callbacks
    bool onFrameCaptureToggle();
    bool onHighResToggle();
    
    bool toggleScene(const int sceneId);
    bool showInterface(const int sceneId);
    
protected: // new
    
    void drawToScreen();
    void drawToFbo();
    void drawFromFbo();
    void drawScenes();
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
    
    // input
    AudioInput              mAudioInput;
    MidiInput               mMidiInput;
    
    MindWave                mMindWave;
    bool                    mEnableMindWave;
    
    OscServer               mOscServer;
    bool                    mEnableOscServer;
    
    KinectController        mKinectController;
    bool                    mEnableKinect;
    
    Vec2f                   mMousePos;
    
    // render
    double                  mLastElapsedSeconds;
    double                  mElapsedSecondsThisFrame;
    MayaCamUI               mMayaCam;
    bool                    mUseMayaCam;
    
    // temp
    typedef vector<Scene*>  SceneList;
    SceneList               mScenes;
    
    // debug
    InfoPanel               mInfoPanel;
    params::InterfaceGl		mParams;
    
    Interface*              mInterface;
    bool                    mIsPresentationMode;
    
    bool                    mIsCapturingVideo;
    qtime::MovieWriter      mMovieWriter;
    
    bool                    mIsCapturingFrames;
    string                  mFrameCapturePath;
    int                     mFrameCaptureCount;
    bool                    mIsCapturingHighRes;
    gl::Fbo                 mFbo;
    float                   mCaptureDuration;
    
    bool                    mSaveNextFrame;
    bool                    mCaptureDebugOutput;
    
    syphonServer            mScreenSyphon;
    bool                    mEnableSyphonServer;
    
    float                   mFpsSendTimer;
};

#endif