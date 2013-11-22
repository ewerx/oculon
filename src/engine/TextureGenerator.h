//
//  TextureGenerator.h
//  Oculon
//
//  Created by Ehsan Rezaie on 11/22/2013.
//
//

#pragma once

#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"

#include "Interface.h"

class TextureGenerator
{
public:
    // shaders
#define TS_SHADERS_TUPLE \
TS_SHADERS_ENTRY( "Kali", "kifs_frag.glsl", SHADER_KALI ) \
TS_SHADERS_ENTRY( "Simplicity", "simplicity_frag.glsl", SHADER_SIMPLICITY ) \
TS_SHADERS_ENTRY( "Voronoi", "voronoi_frag.glsl", SHADER_VORONOI ) \
//TS_SHADERS_ENTRY( "Contour", "contour_tex_frag.glsl", SHADER_NOISE ) \
//end tuple
    
public:
    TextureGenerator();
    virtual ~TextureGenerator();
    
    void setup(const int width, const int height, int type);
    void update(double dt);
    
    void setupInterface( Interface &interface );
    
    ci::gl::Texture getTexture();
    
protected:
    double mElapsedTime;
    float mTimeScale;
    
};