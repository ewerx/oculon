/*
 *  Sol.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __Sol_H__
#define __Sol_H__

#include "Scene.h"
#include "SolFrame.h"

#include "cinder/Cinder.h"
#include "cinder/audio/Input.h"
#include "cinder/Timeline.h"
#include "cinder/gl/Texture.h"

#include <vector>

class TextEntity;

//
// Audio input tests
//
class Sol : public Scene
{
public:
    Sol();
    virtual ~Sol();
    
    // inherited from Scene
    void setup();
    void reset();
    void resize();
    void update(double dt);
    void draw();
    void drawDebug();
    bool handleKeyDown(const ci::app::KeyEvent& keyEvent);
    
    // callbacks
    
protected:
    void setupInterface();
    void setupDebugInterface();
    
    void drawHud();
    
    void loadTextures();
    
private:
    //std::vector<ci::gl::Texture*> mTextures;
    std::vector<SolFrame*> mFrames;
    int mIndex;
    float mFrameRate;
    float mFrameTime;
    int mCurrentSource;
};

#endif // __Sol_H__
