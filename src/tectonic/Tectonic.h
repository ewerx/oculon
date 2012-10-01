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

class SineWave
{
public:
	SineWave( uint32_t freq, float duration );
	~SineWave() {}
	void getData( uint64_t inSampleOffset, uint32_t inSampleCount, ci::audio::Buffer32f *ioBuffer );
private:
	uint32_t mFreq;
	float mDuration;
};

//
// Tectonic
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
    bool bpmTap();
    bool changeDataSource();
    
protected:
    enum eDataSource
    {
        DATASOURCE_QUAKES,
        DATASOURCE_NUKES,
        
        DATASOURCE_COUNT
    };
    
    void setupInterface();
    void setupDebugInterface();
    
    void initEvents(const eDataSource src);
    void clearQuakes();
    void drawEarthMap();
    void drawPoints();
    void drawQuakes();
    void drawHud();
    
    void triggerAll();
    void triggerByBpm(double dt);
    void triggerByTime(double dt);
    
private:
    QuakeData* mData[DATASOURCE_COUNT];
    typedef std::vector<Quake*> QuakeList;
    QuakeList mQuakes;
    QuakeList mActiveQuakes;
    int mDataSource;
    
    struct IsTriggeredQuakeFinished
    {
        bool operator()(Quake* q) const
        {
            return( !q || !q->isTriggered() );
        }
    };
    
    // map
    ci::gl::Texture mEarthDiffuse;
    
    // rendering
    bool mShowMap;
    bool mShowLabels;
    bool mShowAllPoints;
    ci::Color mMarkerColor;
    float mEarthAlpha;
    bool mColorNukesByType;
    
    // audio
    bool mGenerateAudio;
    
    // points
    GLuint mDisplayListPoints;
    
    // simulation
    int mLongitudeOffsetDegrees;
    int mCurrentIndex;
    
    enum eTriggerMode
    {
        TRIGGER_MANUAL,
        TRIGGER_BPM,
        TRIGGER_REALTIME,
        TRIGGER_ALL,
        
        TRIGGER_COUNT
    };
    eTriggerMode mTriggerMode;
    
    float mBpmTriggerTime;
    float mBpm;
    float mBpmTapTimer;
    float mBpmAverage;
    
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
    
    // capture
    bool mIsCapturing;
    
    bool mSendToBinned;
};

#endif // __Tectonic_H__
