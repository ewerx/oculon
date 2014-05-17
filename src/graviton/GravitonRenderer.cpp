//
//  GravitonRenderer.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2/22/2014.
//
//

#include "GravitonRenderer.h"
#include "Utils.h"
#include "cinder/Camera.h"

using namespace ci;
using namespace ci::app;
using namespace std;


GravitonRenderer::GravitonRenderer()
: ParticleRenderer("graviton")
{
    // params
    mAdditiveBlending       = true;
    mPointSize              = 0.6f;
    mColor                  = ColorAf( 0.5f, 0.5f, 0.6f, 0.5f );
    mAudioReactive          = true;
    
    // load textures
    mCurPointTexture        = 0;
    gl::Texture pointTex;
    gl::Texture::Format format;
    format.setWrap( GL_REPEAT, GL_REPEAT );
    
    pointTex = gl::Texture( loadImage( loadResource( "particle_white.png" ) ), format );
    mPointTextures.push_back( make_pair("glow", pointTex) );
    pointTex = gl::Texture( loadImage( loadResource( "glitter.png" ) ), format );
    mPointTextures.push_back( make_pair("solid", pointTex) );
    pointTex = gl::Texture( loadImage( loadResource( "parsec-sparkle.png" ) ), format );
    mPointTextures.push_back( make_pair("sparkle", pointTex) );
}

GravitonRenderer::~GravitonRenderer()
{
    ParticleRenderer::~ParticleRenderer();
}

void GravitonRenderer::setup(int fboSize)
{
    // load shader
    mShader = Utils::loadVertAndFragShaders("graviton_render_vert.glsl",  "graviton_render_frag.glsl");
    
    // setup VBO
    setupVBO(fboSize, GL_POINTS);
}

void GravitonRenderer::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + mName;
    
    interface->gui()->addLabel(mName);
    interface->addParam(CreateBoolParam("audioreactive", &mAudioReactive)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam( "pointsize", &mPointSize )
                         .minValue(0.01f)
                         .maxValue(2.0f)
                         .oscReceiver(oscName));
    
    interface->addParam(CreateColorParam("color", &mColor, kMinColor, kMaxColor)
                        .oscReceiver(oscName));
    vector<string> pointTexNames;
    for( tNamedTexture namedTex : mPointTextures )
    {
        pointTexNames.push_back(namedTex.first);
    }
    interface->addEnum(CreateEnumParam( "point_tex", (int*)(&mCurPointTexture) )
                       .maxValue(pointTexNames.size())
                       .oscReceiver(oscName)
                       .isVertical(), pointTexNames);
}

void GravitonRenderer::draw( PingPongFbo& particlesFbo, const ci::Vec2i& screenSize, const ci::Camera& cam, AudioInputHandler& audioInputHandler )
{
    // pre-render - set state
    gl::pushMatrices();
    gl::setMatrices( cam );
    
    preRender();
    
    // bind textures
    particlesFbo.bindTexture(0);//pos
    particlesFbo.bindTexture(1);//vel
    particlesFbo.bindTexture(2);//info
    
    mPointTextures[mCurPointTexture].second.bind(3);
    
    if (audioInputHandler.hasTexture())
    {
        audioInputHandler.getFbo().bindTexture(4);
    }
    
    // bind shader
    mShader.bind();
    mShader.uniform("displacementMap", 0 );
    mShader.uniform("velocityMap", 1);
    mShader.uniform("pointSpriteTex", 3);
    mShader.uniform("intensityMap", 4);
    mShader.uniform("screenWidth", (float)screenSize.x*2.0f);
    mShader.uniform("spriteWidth", mPointSize);
    mShader.uniform("MV", cam.getModelViewMatrix());
    mShader.uniform("P", cam.getProjectionMatrix());
    mShader.uniform("colorScale", mColor);
    mShader.uniform("audioReactive", mAudioReactive);
    mShader.uniform("gain", audioInputHandler.getGain());
    
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

void GravitonRenderer::preRender()
{
    glPushAttrib( GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT );
    
    gl::enable(GL_TEXTURE_2D);
    
    // setup for point resizing based on position
    gl::enable(GL_POINT_SPRITE);
    gl::enable(GL_PROGRAM_POINT_SIZE_EXT);
    glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
    // when false, point sprites just have one texture coordinate
    // when true, iterates over the point sprite texture...
    // TODO: understand what the difference is!
    // when it is on, the color param from the displacement_vert shaders cannot be applied to the frag color in displacement_frag, it always looks like a gradient between green/yellow instead... have on idea why
    // also don't really know what enabling this actually does so leaving it off
    // until i understand...
    //glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	
    // blend mode
	if(mAdditiveBlending)
    {
        gl::enableAdditiveBlending();
	}
    else
    {
		gl::enableAlphaBlending();
	}
    
    //gl::disableDepthWrite();
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
}

void GravitonRenderer::postRender()
{
    
}
