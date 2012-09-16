/*
 *  SubScene.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#pragma once

class Scene;

class SubScene
{
public:
    SubScene(Scene* scene);
    virtual ~SubScene();

    virtual void setup() = 0;
    virtual void setupInterface() {}
    virtual void setupDebugInterface() {}
    virtual void resize() {}
    virtual void reset() {}
    virtual void update(double dt) = 0;
    virtual void draw() = 0;
    
    bool isActive() const { return mIsActive; }
    
protected:
    Scene*  mParentScene;
    bool    mIsActive;
};
