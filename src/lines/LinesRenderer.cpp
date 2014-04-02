//
//  LinesRenderer.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2/22/2014.
//
//

#include "LinesRenderer.h"
#include "Utils.h"
#include "cinder/Camera.h"

using namespace ci;


LinesRenderer::LinesRenderer()
: ParticleRenderer()
{
    // load textures
    mColorMapTex = gl::Texture( loadImage( app::loadResource( "glitter.png" ) ) );
    
    // params
    mLineWidth = 1.25f;
    mColor = ColorAf(1.0f,1.0f,1.0f,0.025f);
    mAudioReactive = true;
}

LinesRenderer::~LinesRenderer()
{
    ParticleRenderer::~ParticleRenderer();
}

void LinesRenderer::setup(int fboSize)
{
    // load shader
    mShader = Utils::loadVertAndFragShaders("lines_render_vert.glsl", "lines_render_frag.glsl");
    
    // setup VBO
    setupVBO(fboSize, GL_LINES);
}

void LinesRenderer::setupInterface( Interface* interface, const std::string& name )
{
    interface->addParam(CreateFloatParam( "line_width", &mLineWidth )
                         .minValue(0.01f)
                         .maxValue(6.0f));
    
    interface->addParam(CreateColorParam("color", &mColor, kMinColor, ColorA(1.0f,1.0f,1.0f,0.5f))
                         .oscReceiver(name));
    
    interface->addParam(CreateBoolParam("audioreactive", &mAudioReactive));
}

void LinesRenderer::draw( PingPongFbo& particlesFbo, const ci::Vec2i& screenSize, const ci::Camera& cam, AudioInputHandler& audioInputHandler, float gain )
{
    // pre-render - set state
    gl::pushMatrices();
    gl::setMatrices( cam );
    
    preRender();
    
    // bind textures
    particlesFbo.bindTexture(0);//pos
    particlesFbo.bindTexture(1);//vel
    particlesFbo.bindTexture(2);//info
    
    mColorMapTex.bind(3);
    
    if (audioInputHandler.hasTexture())
    {
        audioInputHandler.getFbo().bindTexture(4);
    }
    
    // bind shader
    mShader.bind();
    mShader.uniform("posMap", 0);
    mShader.uniform("velMap", 1);
    mShader.uniform("information", 2);
    mShader.uniform("colorMap", 3);
    mShader.uniform("intensityMap", 4);
    mShader.uniform("gain", gain);
    mShader.uniform("screenWidth", (float)1.0f);
    mShader.uniform("colorBase", mColor);
    mShader.uniform("audioReactive", mAudioReactive);
    
    const float scale = 1000.0f;
    glScalef(scale, scale, scale);
    
    // do magic
    gl::draw( mVboMesh );
    
    // cleanup
    mShader.unbind();
    particlesFbo.unbindTexture();
    mColorMapTex.unbind();
    if (audioInputHandler.hasTexture())
    {
        audioInputHandler.getFbo().unbindTexture();
    }
    
    // post-render - restore state
    glPopAttrib();
    gl::popMatrices();
}

void LinesRenderer::preRender()
{
    glPushAttrib( GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT );
    
    glEnable(GL_TEXTURE_2D);
    
    gl::enableAdditiveBlending();
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    gl::lineWidth(mLineWidth);
}

void LinesRenderer::postRender()
{
    
}
