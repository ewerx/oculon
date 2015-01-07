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
using namespace std;


LinesRenderer::LinesRenderer()
: ParticleRenderer("lines")
{
    // params
    mLineWidth              = 2.0f;
    mColor                  = ColorAf(1.0f,1.0f,1.0f,0.075f);
    mUseColorMap            = false;
    mAudioReactive          = true;
    mAlphaGain              = 3.0f;
    
    // load textures
    mColorMapTex = gl::Texture( loadImage( app::loadResource( "colortex1.jpg" ) ) );
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

void LinesRenderer::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + mName;
    
    interface->gui()->addLabel(getName());
    interface->addParam(CreateFloatParam( "lines/line_width", &mLineWidth )
                         .minValue(0.01f)
                         .maxValue(6.0f)
                        .oscReceiver(oscName));
    
    interface->addParam(CreateBoolParam("lines/audioreactive", &mAudioReactive)
                        .oscReceiver(oscName));
    interface->addParam(CreateColorParam("lines/color", &mColor, kMinColor, ColorA(1.0f,1.0f,1.0f,0.5f))
                         .oscReceiver(oscName));
    
    interface->addParam(CreateFloatParam("lines/alphagain", &mAlphaGain)
                        .minValue(1.0f)
                        .maxValue(10.0f)
                        .oscReceiver(oscName)
                        .midiInput(0, 1, 16));
    interface->addParam(CreateBoolParam("lines/colormap", &mUseColorMap)
                        .oscReceiver(oscName));
    
    // FIXME: MIDI HACK
    mowa::sgui::PanelControl* hiddenPanel = interface->gui()->addPanel();
    hiddenPanel->enabled = false;
    interface->addParam(CreateFloatParam("lines/alpha", &mColor.a)
                        .minValue(0.0f)
                        .maxValue(0.5f)
                        .midiInput(0, 2, 16));
}

void LinesRenderer::draw( PingPongFbo& particlesFbo, const ci::Vec2i& screenSize, const ci::Camera& cam, AudioInputHandler& audioInputHandler )
{
    // pre-render - set state
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    gl::pushMatrices();
    gl::setMatrices( cam );
    
    gl::enable( GL_TEXTURE_2D );
    gl::enable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    glShadeModel( GL_SMOOTH );
    
    gl::enableAdditiveBlending();
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    gl::lineWidth(mLineWidth);
    
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
    mShader.uniform("screenWidth", (float)1.0f);
    mShader.uniform("colorBase", mColor);
    mShader.uniform("gain", audioInputHandler.getGain() * mAlphaGain);
    mShader.uniform("audioReactive", mAudioReactive);
    mShader.uniform("useColorMap", mUseColorMap);
    
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
