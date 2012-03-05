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

#include "cinder/app/AppBasic.h"
#include "cinder/audio/Input.h"
#include "cinder/Camera.h"
#include "cinder/params/Params.h"
#include "cinder/MayaCamUI.h"
#include "cinder/qtime/MovieWriter.h"
#include "cinder/gl/Fbo.h"
#include "AudioInput.h"
#include "MidiInput.h"
#include "MindWave.h"
#include "OscServer.h"
#include "InfoPanel.h"
#include <vector>
#include "cinderSyphon.h"

// fwd decl
class Scene;

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
    inline AudioInput& getAudioInput()         { return mAudioInput; }
    inline MidiInput& getMidiInput()           { return mMidiInput; }
    inline MindWave& getMindWave()             { return mMindWave; }
    
    inline const Vec2f& getMousePos()          { return mMousePos; }
    inline InfoPanel& getInfoPanel()           { return mInfoPanel; }
    inline params::InterfaceGl& getParams()    { return mParams; }
    
    inline bool isPresentationMode() const     { return mIsPresentationMode; }
    inline void setUseMayaCam(bool use)        { mUseMayaCam = use; }
    
    void setCamera( const Vec3f& eye, const Vec3f& look, const Vec3f& up );
    inline const Camera& getCamera() const     { return (mUseMayaCam ? mMayaCam.getCamera() : mCam);  }
    
    inline double getElapsedSecondsThisFrame() const  { return mElapsedSecondsThisFrame; }
    
    //TODO: hack
    Scene* getScene(const int index)            { return ( index < mScenes.size() ) ? mScenes[index] : NULL ; }
    
    void enableFrameCapture( bool enable );
    
protected: // new
    
    void setupScenes();
    void addScene(Scene* newScene, bool startActive =false);
    void drawInfoPanel();
    void setPresentationMode( bool enabled );
    
    void startVideoCapture( bool useDefaultPath =true );
    void stopVideoCapture();
    
    
private: // members
    // input
    AudioInput              mAudioInput;
    MidiInput               mMidiInput;
    MindWave                mMindWave;
    bool                    mEnableMindWave;
    Vec2f                   mMousePos;
    OscServer               mOscServer;
    bool                    mEnableOscServer;
    
    // render
    double                  mLastElapsedSeconds;
    double                  mElapsedSecondsThisFrame;
    CameraPersp				mCam;
    MayaCamUI               mMayaCam;
    bool                    mUseMayaCam;
    
    // temp
    typedef vector<Scene*>  SceneList;
    SceneList               mScenes;
    
    // debug
    InfoPanel               mInfoPanel;
    params::InterfaceGl		mParams;
    
    bool                    mIsPresentationMode;
    
    bool                    mIsCapturingVideo;
    qtime::MovieWriter      mMovieWriter;
    
    bool                    mIsCapturingFrames;
    string                  mFrameCapturePath;
    int                     mFrameCaptureCount;
    bool                    mIsCapturingHighRes;
    gl::Fbo                 mFbo;
    
    bool                    mSaveNextFrame;
    
    syphonServer            mScreenSyphon;
    bool                    mEnableSyphonServer;
};

#endif