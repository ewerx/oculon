/*
 *  Triangle.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#pragma once

#include "SubScene.h"
#include "cinder/Timeline.h"
#include "cinder/Color.h"
#include <boost/multi_array.hpp>

class Scene;

class Triangle : public SubScene
{
public:
    Triangle(Scene* scene);
    virtual ~Triangle();

    void setup();
    void setupInterface();
    //void setupDebugInterface();
    void resize();
    void reset();
    void update(double dt);
    void draw();
    
private:
    
    ci::Vec2f mTop;
    ci::Vec2f mLeft;
    ci::Vec2f mRight;
    
    float mScale;
    
    ci::ColorA mColor;

};
