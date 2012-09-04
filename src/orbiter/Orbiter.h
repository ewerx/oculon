/*
 *  Orbiter.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-19.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __ORBITER_H__
#define __ORBITER_H__

#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include <vector>

#include "Scene.h"
#include "MidiMap.h"

using std::vector;

class Body;
class Sun;
class TextEntity;
class Star;

//
// An orbital simulator
//
class Orbiter : public Scene
{
public:
    Orbiter();
    virtual ~Orbiter();
    
    // inherited from Scene
    void setup();
    void reset();
    void resize();
    void update(double dt);
    void draw();
    bool handleKeyDown(const ci::app::KeyEvent& keyEvent);
    const Camera& getCamera();
    
    // callbacks
    void handleGravityChange(MidiEvent midiEvent);
    void handleTimeScaleChange(MidiEvent midiEvent);
    bool prevTarget();
    bool nextTarget();
    
    void createSystem( Star* star );
    
    
    // HACK
    enum eCamType
    {
        CAM_MANUAL,
        CAM_FOLLOW,
        CAM_BINNED,
        CAM_CATALOG,
        
        CAM_COUNT
    };
    eCamType getCamType() { return mCamType; }
    
protected:// from Scene
    void setupInterface();
    void setupDebugInterface();
    
private:
    void setupMidiMapping();
    
    void updateAudioResponse();
    void updateCam();
    
    void setupHud();
    void updateHud();
    void drawHud();
    void drawHudWaveformAnalyzer(float left, float top, float width, float height);
    void drawHudSpectrumAnalyzer(float left, float top, float width, float height);
    
    void removeBodies();
    void createSolSystem();
    
private:
    //Vec3f           mLightDirection;
    
    typedef vector<Body*> BodyList;
    BodyList            mBodies;
    
    enum { NUM_PLANETS = 8 };
    ImageSourceRef      mTextures[NUM_PLANETS];
    
    Sun*                mSun;
    int                 mFollowTargetIndex;
    Body*               mFollowTarget;
    CameraPersp         mCam;
    Matrix44d           mScaleMatrix;
    
    
    eCamType        mCamType;
    
    double          mElapsedTime;
    double          mTimeScale;
    double          mDrawScale;
    double          mGravityConstant;
    
    Quatd           mCameraRotation;
    
    MidiMap         mMidiMap;
    
    enum eTextBoxLocations
    {
        TB_TOP_LEFT,
        TB_TOP_RIGHT,
        TB_BOT_LEFT,
        TB_BOT_RIGHT,
        
        TB_COUNT
    };
    TextEntity*     mTextBox[TB_COUNT];
    
public:
    static double   sDefaultTimeScale;
    static double   sDefaultGravityConstant;
    static double   sDrawScale;
    static float    sMaxRadiusMultiplier;
    static int      sNumFramesToAvgFft;
    static bool     sUseSmoothLines;
    static bool     sUseTriStripLine;
    static int      sMinTrailLength;
    static float    sTrailWidth;
    static bool     sDrawRealSun;
    static float    sPlanetGrayScale;
};

#endif // __ORBITER_H__
