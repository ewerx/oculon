//
//  DustRenderer.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-04-02.
//
//

#include "DustRenderer.h"
#include "Utils.h"
#include "cinder/Camera.h"

using namespace ci;
using namespace ci::app;
using namespace std;


DustRenderer::DustRenderer()
: ParticleRenderer("dust")
{
    // params
    mPointSize = 1.25f;
    mColor = ColorAf(1.0f,1.0f,1.0f,1.0f);
    mAudioReactive = true;
    mAdditiveBlend = false;
 
    // load textures
    gl::Texture tex;
    gl::Texture::Format format;
    format.setWrap( GL_REPEAT, GL_REPEAT );
    
    mCurPointTexture = 0;
    mPointTextures.push_back( make_pair("glow", gl::Texture( loadImage( loadResource( "particle_white.png" ) ), format )) );
    mPointTextures.push_back( make_pair("glow2", gl::Texture( loadImage( loadResource( "particle_white2.png" ) ), format )) );
    mPointTextures.push_back( make_pair("solid", gl::Texture( loadImage( loadResource( "glitter.png" ) ), format )) );
    mPointTextures.push_back( make_pair("donut", gl::Texture( loadImage( loadResource( "donut64.png" ) ), format )) );
    mPointTextures.push_back( make_pair("sparkle", gl::Texture( loadImage( loadResource( "parsec-sparkle.png" ) ), format )) );
    
    mCurColorMap = 0;
    
    mColorMaps.push_back( make_pair("none", gl::Texture( loadImage( loadResource( "blank-8.png" ) ), format )) );
    tex = gl::Texture( loadImage( loadResource( "colortex1-256.png" ) ), format );
    mColorMaps.push_back( make_pair("fire", tex) );
    tex = gl::Texture( loadImage( loadResource( "colortex2-256.png" ) ), format );
    mColorMaps.push_back( make_pair("ice", tex) );
    tex = gl::Texture( loadImage( loadResource( "colortex3-256.png" ) ), format );
    mColorMaps.push_back( make_pair("red-blue", tex) );
    tex = gl::Texture( loadImage( loadResource( "colortex4-256.png" ) ), format );
    mColorMaps.push_back( make_pair("golden", tex) );
    
}

DustRenderer::~DustRenderer()
{
    ParticleRenderer::~ParticleRenderer();
}

void DustRenderer::setup(int fboSize)
{
    // load shader
    mShader = Utils::loadVertAndFragShaders("dust_render_vert.glsl", "dust_render_frag.glsl");
    
    // setup VBO
    setupVBO(fboSize, GL_POINTS);
}

void DustRenderer::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + mName;
    
    interface->gui()->addLabel(getName());
    
    // point sprite
    interface->addParam(CreateFloatParam( "dust/point_size", &mPointSize )
                        .minValue(0.01f)
                        .maxValue(6.0f)
                        .oscReceiver(oscName));
    
    vector<string> pointTexNames;
    for( tNamedTexture namedTex : mPointTextures )
    {
        pointTexNames.push_back(namedTex.first);
    }
    interface->addEnum(CreateEnumParam( "dust/point_tex", (int*)(&mCurPointTexture) )
                       .maxValue(pointTexNames.size())
                       .oscReceiver(oscName)
                       .isVertical(), pointTexNames);
    
    // color
    interface->addParam(CreateColorParam("dust/color", &mColor, kMinColor, ColorA(1.0f,1.0f,1.0f,0.5f))
                        .oscReceiver(oscName));
    
    vector<string> colorMapNames;
    for( tNamedTexture namedTex : mColorMaps )
    {
        colorMapNames.push_back(namedTex.first);
    }
    interface->addEnum(CreateEnumParam( "dust/colormap", (int*)(&mCurColorMap) )
                       .maxValue(colorMapNames.size())
                       .oscReceiver(oscName)
                       .isVertical(), colorMapNames);
    
    interface->addParam(CreateBoolParam("dust/additive", &mAdditiveBlend));
    interface->addParam(CreateBoolParam("dust/audioreactive", &mAudioReactive));
}

void DustRenderer::draw( PingPongFbo& particlesFbo, const ci::Vec2i& screenSize, const ci::Camera& cam, AudioInputHandler& audioInputHandler )
{
    // pre-render - set state
    glPushAttrib( GL_ALL_ATTRIB_BITS );
    
    gl::enable(GL_TEXTURE_2D);
    
    // point rendering
    gl::enable(GL_POINT_SPRITE);
    gl::enable(GL_PROGRAM_POINT_SIZE_EXT);
    glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    
    if (mAdditiveBlend)
    {
        gl::enableAdditiveBlending();
    }
    else
    {
        gl::enableAlphaBlending();
    }
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    gl::pushMatrices();
    gl::setMatrices( cam );
    gl::setViewport(Area(Vec2i(0,0), screenSize));
    
    // bind textures
    particlesFbo.bindTexture(0);//pos
    particlesFbo.bindTexture(1);//vel
    particlesFbo.bindTexture(2);//info
    
    mPointTextures[mCurPointTexture].second.bind(3);
    mColorMaps[mCurColorMap].second.bind(5);
    
    if (audioInputHandler.hasTexture())
    {
        audioInputHandler.getFbo().bindTexture(4);
    }
    
    // bind shader
    mShader.bind();
    mShader.uniform("posMap", 0);
    mShader.uniform("velMap", 1);
    mShader.uniform("information", 2);
    mShader.uniform("spriteTex", 3);
    mShader.uniform("colorMap", 5);
    mShader.uniform("intensityMap", 4);
    mShader.uniform("spriteWidth", mPointSize);
    mShader.uniform("gain", audioInputHandler.getGain());
    mShader.uniform("screenWidth", (float)screenSize.x);
    mShader.uniform("colorBase", mColor);
    mShader.uniform("audioReactive", mAudioReactive);
    
    // do magic
    gl::draw( mVboMesh );
    
    // cleanup
    mShader.unbind();
    particlesFbo.unbindTexture();
    mPointTextures[mCurPointTexture].second.unbind();
    if (audioInputHandler.hasTexture())
    {
        audioInputHandler.getFbo().unbindTexture();
    }
    
    // post-render - restore state
    glPopAttrib();
    gl::popMatrices();
}
