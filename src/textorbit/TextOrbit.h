//
// TextOrbit.h
// Oculon
//
// Created by Ehsan on 12-12-28.
// Copyright 2011 ewerx. All rights reserved.
//


#pragma once


#include "Scene.h"
#include "SplineCam.h"
#include "Character.h"
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Surface.h"
#include <deque>


//
// TextOrbit
//
class TextOrbit : public Scene
{
public:
    TextOrbit();
    virtual ~TextOrbit();
    
    // inherited from Scene
    void setup();
    //void reset();
    //void resize();
    void update(double dt);
    void draw();
    const ci::Camera& getCamera();
    
protected:// from Scene
    void setupInterface();
    //void setupDebugInterface();
    
private:
    void addChar( char c );
    void updateAudioResponse();
    
private:
    ci::gl::TextureFontRef	mTextureFont;
    
    std::vector<Character> mCharacters;
    
    // params

};

