/*
 *  Tectonic.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __Tectonic_H__
#define __Tectonic_H__

#include "Scene.h"
#include "MotionBlurRenderer.h"
#include "Quake.h"

#include "cinder/Cinder.h"
#include "cinder/audio/Input.h"
#include "cinder/Timeline.h"
#include "cinder/gl/Texture.h"

#include <vector>

//class Quake;
class QuakeData;
class TextEntity;

//
// Audio input tests
//
class Tectonic : public Scene
{
public:
    Tectonic();
    virtual ~Tectonic();
    
    // inherited from Scene
    void setup();
    void reset();
    void resize();
    void update(double dt);
    void draw();
    void drawDebug();
    bool handleKeyDown(const ci::app::KeyEvent& keyEvent);
    
    // callbacks
    bool triggerNextQuake();
    
protected:
    void setupInterface();
    void setupDebugInterface();
    
    void initQuakes();
    void clearQuakes();
    void drawEarthMap();
    void drawQuakes();
    void drawHud();
    
    void triggerAll();
    void triggerByBpm(double dt);
    void triggerByTime(double dt);
    
private:
    QuakeData* mData;
    typedef std::vector<Quake*> QuakeList;
    QuakeList mQuakes;
    QuakeList mActiveQuakes;
    
    struct IsTriggeredQuakeFinished
    {
        bool operator()(Quake* q) const
        {
            return( !q || !q->isTriggered() );
        }
    };
    
    // map
    ci::gl::Texture mEarthDiffuse;
    int mLongitudeOffsetDegrees;
    
    int mCurrentIndex;
    
    enum eTriggerMode
    {
        TRIGGER_ALL,
        TRIGGER_MANUAL,
        TRIGGER_REALTIME,
        TRIGGER_BPM,
        
        TRIGGER_COUNT
    };
    eTriggerMode mTriggerMode;
    
    float mBpmTriggerTime;
    float mBpm;
    
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

#endif // __Tectonic_H__
