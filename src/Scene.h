/*
 *  Scene.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-17.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __SCENE_H__
#define __SCENE_H__

#include "cinder/Cinder.h"
#include "cinder/params/Params.h"
#include "cinder/app/KeyEvent.h"

// fwd decl
class OculonApp;


class Scene
{
public:
    Scene();
    virtual ~Scene();
    
    void init(OculonApp* app)  { mApp = app; }
    
    virtual void setup() {}
    virtual void setupParams(ci::params::InterfaceGl& params) {}
    virtual void reset() {}
    virtual void update(double dt) {}
    virtual void draw() {}
    virtual bool handleKeyDown(const ci::app::KeyEvent& keyEvent) { return false; }
    
    OculonApp* getApp();
    
    bool isActive() const           { return mIsActive; }
    bool isVisible() const          { return mIsVisible; }
    
    void setActive(bool active)     { mIsActive = active; }
    void setVisible(bool visible)   { mIsVisible = visible; }
    
    void toggleActiveVisible()      { mIsActive = !mIsActive; mIsVisible = !mIsVisible; }
    
protected:
    OculonApp* mApp;//TODO: fix this dependency
    
    bool mIsActive;
    bool mIsVisible;
    
};

#endif // __SCENE_H__
