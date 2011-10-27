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
    AudioInput& getAudioInput()         { return mAudioInput; }
    MidiInput& getMidiInput()           { return mMidiInput; }
    const Vec2f& getMousePos()          { return mMousePos; }
    InfoPanel& getInfoPanel()           { return mInfoPanel; }
    
protected: // new
    
    void setupScenes();
    
    void drawWaveform   ( audio::PcmBuffer32fRef pcmBufferRef );
    void drawFft        ( std::shared_ptr<float> fftDataRef );
    void drawInfoPanel();
    
private: // members
    // input
    AudioInput              mAudioInput;
    MidiInput               mMidiInput;
    Vec2f                   mMousePos;
    
    // render
    double                  mLastElapsedSeconds;
    CameraPersp				mCam;
    MayaCamUI               mMayaCam;
    
    // temp
    vector<Scene*>          mScenes;
    
    // debug
    InfoPanel               mInfoPanel;
    bool                    mRenderInfoPanel;
    params::InterfaceGl		mParams;
};

#endif