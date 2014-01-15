//
//  Oscillator.h
//  Oculon
//
//  Created by Ehsan Rezaie on 1/14/2014.
//
//

#pragma once

#include "AudioInputHandler.h"
#include "Scene.h"
#include "cinder/Cinder.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Timeline.h"
#include <vector>

//
// Oscillator
//
class Oscillator : public Scene
{
public:
    Oscillator();
    virtual ~Oscillator();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    
protected:// from Scene
    void setupInterface();
    
private:
    void shaderPreDraw();
    void drawShaderOutput();
    void shaderPostDraw();

private:
    ci::gl::GlslProg    mShader;
    
    struct tWaveParams
    {
        ci::Anim<ci::ColorAf>   mColor;
        float                   mElapsedTime;
        float                   mTimeScale;
        float                   mOffset;
        ci::Anim<float>         mAmplitude;
        ci::Anim<float>         mFrequency;
    };
    enum { MAX_WAVES = 16 };
    tWaveParams mWaveParams[MAX_WAVES];
};