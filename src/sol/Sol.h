/*
 *  Sol.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __Sol_H__
#define __Sol_H__

#include "Scene.h"
#include "SolFrame.h"

#include "cinder/Cinder.h"
#include "cinder/audio/Input.h"
#include "cinder/Timeline.h"
#include "cinder/gl/Texture.h"

#include <vector>

class TextEntity;

//
// Audio input tests
//
class Sol : public Scene
{
public:
    Sol();
    virtual ~Sol();
    
    // inherited from Scene
    void setup();
    void reset();
    void resize();
    void update(double dt);
    void draw();
    void drawDebug();
    bool handleKeyDown(const ci::app::KeyEvent& keyEvent);
    
    // callbacks
    bool nextSource();
    
protected:
    void setupInterface();
    void setupDebugInterface();
    
    void setupHud();
    void updateHud();
    void drawHud();
    
    void initFrames();
    void setNextFrame();
    
private:
    // frames
    typedef std::vector<SolFrame*> FrameList;
    FrameList mFrames;
    int mIndex;
    float mFrameRate;
    float mFrameTime;
    int mCurrentSource;
    
    // display
    ci::gl::Texture mMaskTexture;
    bool mDrawMask;
    bool mMultiSource;
    
    // playback
    enum ePlaybackMode
    {
        PLAYBACK_FORWARD,
        PLAYBACK_REVERSE,
        PLAYBACK_RANDOM,
        
        PLAYBACK_COUNT
    };
    ePlaybackMode mPlaybackMode;
    bool mPingPong;
    
    // hud
    enum eTextBoxLocations
    {
        TB_TOP_LEFT,
        TB_TOP_RIGHT,
        TB_BOT_LEFT,
        TB_BOT_RIGHT,
        
        TB_COUNT
    };
    TextEntity*     mTextBox[TB_COUNT];
};

#endif // __Sol_H__
