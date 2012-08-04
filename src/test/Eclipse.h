/*
 *  Eclipse.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#pragma once

#include "cinder/Timeline.h"
#include "cinder/PolyLine.h"
#include "cinder/Rect.h"
#include "cinder/gl/Texture.h"
#include "Constants.h"

class Scene;

//TODO: refactor as SubScene
class Eclipse
{
public:
    Eclipse(Scene* scene);
    virtual ~Eclipse();

    void setup();
    void setupInterface();
    void setupDebugInterface();
    void resize();
    void reset();
    void update(double dt);
    void draw();
    
private:
    void drawLines();
    void drawRects();
    void drawCubes();
    void drawRowRects(const float x1, const float y1, const float thickness);
    void drawRowLines(const float x1, const float y1, const int rectIndex);
    float getThicknessValue(const int baseIndex);
    
    bool resetSnakes();
    void updateSnakes(double dt);
    void drawSnakes();
    
    ci::Vec2f getPoint();
    
private:
    Scene*  mParentScene;
    
    bool mShowTestBackground;
    ci::gl::Texture mTestBackground;
    
    enum
    {
        ROW1_RECTS = 6,
        ROW2_RECTS = 4,
        ROW3_RECTS = 2,
        NUM_RECTS = 12,
        NUM_CUBES = 6,
        NUM_ROWS = 3,
        NUM_HLINES = 4,
        NUM_VLINES = 7
    };
    int mRectsInRow[NUM_ROWS];
    int mLinesInRow[NUM_HLINES];
    int mMinColForRow[NUM_HLINES];
    int mMaxColForRow[NUM_HLINES];
    float mXMin[NUM_HLINES];
    float mXMax[NUM_HLINES];
    float mYMin[NUM_VLINES];
    float mYMax[NUM_VLINES];
    
    ci::Rectf mFaces[NUM_RECTS];
    int mDim;
    int mXOffset;
    int mYOffset;
    
    enum eMode
    {
        MODE_LINES,
        MODE_RECTS,
        MODE_CUBES,
        MODE_SNAKES,
        
        MODE_COUNT
    };
    eMode mMode;
    
    ci::Anim<float> mThickness[NUM_CUBES];
    float mMaxThickness;
    bool mInverse;
    bool mDrawInnerLines;
    
    bool mUseFftData;
    bool mRandomizeFaces;
    bool mRandomizeSignal;
    float mSignalScale;
    
    struct tSnake
    {
        ci::Vec2f mHead;
        //ci::Vec2f mMid;
        //ci::Vec2f mTail;
        
        //ci::Path2d mPath;
        ci::Vec2f mDir;
        float mSpeed;
        float mThickness;
        //float mLength;
    };
    std::vector<tSnake> mSnakes;
    int mNumSnakes;
    
    tSnake mHSnakes[NUM_HLINES];
    tSnake mVSnakes[NUM_VLINES];
    
    float mSnakeLength;
    
    float mMinSnakeSpeed;
    float mMaxSnakeSpeed;
    float mSnakeSpeedMultiplier;
    
    //bool mSnakeRandomStart;
    bool mSnakeRandomSpeed;
    enum eSnakeDir
    {
        SNAKEDIR_UNISON,
        SNAKEDIR_ALTERNATE,
        SNAKEDIR_RANDOM,
        
        SNAKEDIR_COUNT
    };
    eSnakeDir mSnakeDirMode;
    bool mSnakeRounded;
    
    ci::ColorAf mColor;
};
