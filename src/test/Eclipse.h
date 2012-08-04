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
    void drawCubes();
    
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
        ROWS = 3
    };
    
    ci::Rectf mFaces[NUM_RECTS];
    int mDim;
    int mXOffset;
    int mYOffset;
    
    enum eMode
    {
        MODE_LINES,
        MODE_CUBES,
        
        MODE_COUNT
    };
    eMode mMode;
};
