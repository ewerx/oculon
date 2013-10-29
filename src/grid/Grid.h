//
// Grid.h
// Oculon
//
// Created by Ehsan on 12-12-28.
// Copyright 2011 ewerx. All rights reserved.
//


#pragma once


#include "AudioInputHandler.h"
#include "Scene.h"
#include "MotionBlurRenderer.h"
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Surface.h"
#include "cinder/Timeline.h"
#include <list>


// CONSTANTS
#define GRID_WIDTH 35
#define GRID_HEIGHT 9

//
// Grid
//
class Grid : public Scene
{
public:
    Grid();
    virtual ~Grid();
    
    // inherited from Scene
    void setup();
    void reset();
    //void resize();
    void update(double dt);
    void draw();
    void drawDebug();
    
    
protected:// from Scene
    void setupInterface();
    void setupDebugInterface();
    
private:
    void updateAudioResponse();
    
    void drawScene();
    void drawFromDynamicTexture();
    
    void updatePixels();
    void drawPixels();
    
    
    struct tParticle
    {
        ci::Anim<ci::Vec2f> mPos;
        ci::Anim<ci::Vec2f> mVel;
        ci::Anim<float> mAlpha;
        ci::Anim<int> mWidth;
        ci::Anim<int> mHeight;
        bool mExpand;
        
        tParticle( ci::Vec2f pos, ci::Vec2f vel, int width, int height ) : mPos(pos), mVel(vel), mAlpha(1.0f), mWidth(width), mHeight(height) , mExpand(false) {}
        tParticle() : mPos(ci::Vec2f::zero()), mVel(ci::Vec2f::zero()), mAlpha(1.0f), mExpand(false) {}
    };
    void updateParticles(double dt);
    void drawParticles();
    tParticle spawnParticle(int type =PARTICLEMODE_COUNT);
    
    bool setColorScheme();
    
private:
    // pixelate
    ci::gl::GlslProg			mShaderPixelate;
    
    // motion blur
    MotionBlurRenderer          mMotionBlurRenderer;
    bool                        mMotionBlurEnabled;
    
    // pixel control
    struct tPixel
    {
        float mValue;
        float mFreq;
    };
    tPixel                      mPixels[GRID_WIDTH][GRID_HEIGHT];
    
    // audio response
    AudioInputHandler           mAudioInputHandler;
    
    // params
#define GRIDMODE_TUPLE \
GRIDMODE_ENTRY( "Shapes", GRIDMODE_MOTION ) \
GRIDMODE_ENTRY( "Pixels", GRIDMODE_PIXELS ) \
GRIDMODE_ENTRY( "Particles", GRIDMODE_PARTICLES ) \
GRIDMODE_ENTRY( "Snake", GRIDMODE_SNAKE ) \
//end tuple
    
    enum eGridMode
    {
#define GRIDMODE_ENTRY( nam, enm ) \
enm,
        GRIDMODE_TUPLE
#undef  GRIDMODE_ENTRY
        
        GRIDMODE_COUNT
    };
    eGridMode   mGridMode;
    
    // colors
#define COLORSCHEME_TUPLE \
COLORSCHEME_ENTRY( "Red", COLORSCHEME_RED ) \
COLORSCHEME_ENTRY( "Blue", COLORSCHEME_BLUE ) \
COLORSCHEME_ENTRY( "Turq", COLORSCHEME_TURQ ) \
COLORSCHEME_ENTRY( "Green", COLORSCHEME_GREEN ) \
COLORSCHEME_ENTRY( "Yellow", COLORSCHEME_YELLOW ) \
//end tuple
    
    enum eColorScheme
    {
#define COLORSCHEME_ENTRY( nam, enm ) \
enm,
        COLORSCHEME_TUPLE
#undef  COLORSCHEME_ENTRY
        
        COLORSCHEME_COUNT
    };
    eColorScheme    mColorScheme;
    
    ci::Anim<ci::ColorAf>    mColor1;
    ci::Anim<ci::ColorAf>    mColor2;
    ci::Anim<ci::ColorAf>    mColor3;

    float           mLowPassSplit;
    float           mHighPassSplit;
    
    int             mGroupCols;
    int             mGroupRows;
    
    
    // PARTICLES
    std::list<tParticle> mParticles;
    int mNumParticles;
    tParticle mEmitter;
    
    float mParticleDecay;
    float mParticleSpeed;
    float mParticleSpawnRate;
    float mParticleSpawnTime;
    
#define PARTICLESPAWN_TUPLE \
PARTICLESPAWN_ENTRY( "Manual", PARTICLESPAWN_MANUAL ) \
PARTICLESPAWN_ENTRY( "Time", PARTICLESPAWN_TIME ) \
PARTICLESPAWN_ENTRY( "Audio", PARTICLESPAWN_AUDIO ) \
//end tuple
    
    enum eParticleSpawn
    {
#define PARTICLESPAWN_ENTRY( nam, enm ) \
enm,
        PARTICLESPAWN_TUPLE
#undef  PARTICLESPAWN_ENTRY
        
        PARTICLESPAWN_COUNT
    };
    eParticleSpawn   mParticleSpawn;

#define PARTICLEMODE_TUPLE \
PARTICLEMODE_ENTRY( "Spread", PARTICLEMODE_SPREAD ) \
PARTICLEMODE_ENTRY( "CenterH", PARTICLEMODE_CENTER ) \
PARTICLEMODE_ENTRY( "Rain", PARTICLEMODE_RAIN ) \
PARTICLEMODE_ENTRY( "CenterV", PARTICLEMODE_CENTERV ) \
PARTICLEMODE_ENTRY( "Sides", PARTICLEMODE_SIDES ) \
PARTICLEMODE_ENTRY( "Strips", PARTICLEMODE_STRIPS ) \
//end tuple
    
    enum eParticleMode
    {
#define PARTICLEMODE_ENTRY( nam, enm ) \
enm,
        PARTICLEMODE_TUPLE
#undef  PARTICLEMODE_ENTRY
        
        PARTICLEMODE_COUNT
    };
    eParticleMode   mParticleMode;
    
    int mParticleWidth;
    int mParticleHeight;
    bool mParticleRandomSize;
    int mSpawnAmount;
    
    bool mParticleExpand;
    bool triggerSpawn(int type);
    bool triggerBox(int pos);
    void triggerOneBox(int pos);
    bool mMirrorTriggers;
    
    int mNumVSnakes;
    int mNumHSnakes;
    std::vector<tParticle>  mVSnakes;
    std::vector<tParticle>  mHSnakes;
    float mSnakeLength;
    
    void updateSnakes(double dt);
    void drawSnakes();
    bool resetSnakes();
    
    float mSecondaryColorRate;
};

