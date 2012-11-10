/*
 *  Circles.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#pragma once

#include "SubScene.h"
#include "cinder/TriMesh.h"
#include "cinder/gl/Vbo.h"

class Scene;

class Circles : public SubScene
{
public:
    Circles(Scene* scene);
    virtual ~Circles();

    void setup();
    void setupInterface();
    void setupDebugInterface();
    void resize();
    void reset();
    void update(double dt);
    void draw();
    
private:
    int mRows;
    int mCols;
    int mLayers;
    
    float mSpacing;
    float mRadius;
    
    float mSignalScale;
    
    ci::ColorA mColorPrimary;
    ci::ColorA mColorSecondary;

};
