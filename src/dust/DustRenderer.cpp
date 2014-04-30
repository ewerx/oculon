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


DustRenderer::DustRenderer()
: ParticleRenderer()
{
    // load textures
    mColorMapTex    = gl::Texture( loadImage( app::loadResource( "colortex1.jpg" ) ) );
    mSpriteTex      = gl::Texture( loadImage( app::loadResource( "glitter.png" ) ) );
    
    // params
    mPointSize = 1.25f;
    mColor = ColorAf(1.0f,1.0f,1.0f,1.0f);
    mAudioReactive = false;
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

void DustRenderer::setupInterface( Interface* interface, const std::string& name )
{
    interface->addParam(CreateFloatParam( "point_size", &mPointSize )
                        .minValue(0.01f)
                        .maxValue(6.0f)
                        .oscReceiver(name));
    
    interface->addParam(CreateColorParam("color", &mColor, kMinColor, ColorA(1.0f,1.0f,1.0f,0.5f))
                        .oscReceiver(name));
    
    interface->addParam(CreateBoolParam("audioreactive", &mAudioReactive));
}

void DustRenderer::draw( PingPongFbo& particlesFbo, const ci::Vec2i& screenSize, const ci::Camera& cam, AudioInputHandler& audioInputHandler, float gain )
{
    // pre-render - set state
    gl::pushMatrices();
    gl::setMatrices( cam );
    gl::setViewport(Area(Vec2i(0,0), screenSize));
    
    preRender();
    
    // bind textures
    particlesFbo.bindTexture(0);//pos
    particlesFbo.bindTexture(1);//vel
    particlesFbo.bindTexture(2);//info
    
    mSpriteTex.bind(3);
    //mColorMapTex.bind(4);
    
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
    //mShader.uniform("colorMap", 3);
    mShader.uniform("intensityMap", 4);
    mShader.uniform("spriteWidth", mPointSize);
    mShader.uniform("gain", gain);
    mShader.uniform("screenWidth", (float)screenSize.x);
    mShader.uniform("colorBase", mColor);
    mShader.uniform("audioReactive", mAudioReactive);
    
    //const float scale = 1.0f;
    //glScalef(scale, scale, scale);
    //glScalef(screenSize.x / 512.0f , screenSize.y / 512.0f, 1.0f);
    
    // do magic
    gl::draw( mVboMesh );
    
    // cleanup
    mShader.unbind();
    particlesFbo.unbindTexture();
    mSpriteTex.unbind();
    if (audioInputHandler.hasTexture())
    {
        audioInputHandler.getFbo().unbindTexture();
    }
    
    // post-render - restore state
    glPopAttrib();
    gl::popMatrices();
}

void DustRenderer::preRender()
{
    glPushAttrib( GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT );
    
    gl::enable(GL_TEXTURE_2D);
    
    // point rendering
    gl::enable(GL_POINT_SPRITE);
    gl::enable(GL_PROGRAM_POINT_SIZE_EXT);
    glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    
    gl::enableAlphaBlending();
    gl::enableDepthRead();
    gl::enableDepthWrite();
}

void DustRenderer::postRender()
{
    
}
