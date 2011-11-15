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
#include "AudioInput.h"
#include "MidiInput.h"
#include "MindWave.h"
#include "InfoPanel.h"
#include <vector>

// fwd decl
class Scene;

using namespace ci;
using namespace ci::app;
using std::vector;

// main app
//
class OculonApp : public AppBasic 
{
public: // cinder interface
    void prepareSettings( Settings *settings );
	void setup();
	void update();
    void resize( ResizeEvent event );
    
    void mouseMove( MouseEvent event );
	void mouseDrag( MouseEvent event );
    void mouseDown( MouseEvent event );	
    void mouseUp( MouseEvent event ) {};
    void keyDown( KeyEvent event );

	void draw();
    
public: // new
    inline AudioInput& getAudioInput()         { return mAudioInput; }
    inline MidiInput& getMidiInput()           { return mMidiInput; }
    inline MindWave& getMindWave()             { return mMindWave; }
    
    inline const Vec2f& getMousePos()          { return mMousePos; }
    inline InfoPanel& getInfoPanel()           { return mInfoPanel; }
    
    inline bool isPresentationMode() const     { return mIsPresentationMode; }
    inline void setUseMayaCam(bool use)        { mUseMayaCam = use; }
    
    void setCamera( const Vec3f& eye, const Vec3f& look, const Vec3f& up );
    inline const Camera& getCamera() const     { return (mUseMayaCam ? mMayaCam.getCamera() : mCam);  }
    
    inline double getElapsedSecondsThisFrame() const  { return mElapsedSecondsThisFrame; }
    
protected: // new
    
    void setupScenes();
    void drawInfoPanel();
    void setPresentationMode( bool enabled );
    
private: // members
    // input
    AudioInput              mAudioInput;
    MidiInput               mMidiInput;
    MindWave                mMindWave;
    bool                    mEnableMindWave;
    Vec2f                   mMousePos;
    
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
};

#endif