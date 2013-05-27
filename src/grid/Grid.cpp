//
//  Grid.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2012-12-28.
//
//


#include "Grid.h"
#include "Interface.h"
#include "OculonApp.h"
#include "Resources.h"
#include "cinder/Camera.h"
#include "cinder/ImageIo.h"
#include "cinder/app/AppBasic.h"

using namespace ci;
using namespace ci::gl;
using namespace ci::app;
using namespace std;

#define VTF_FBO_SIZE 640
#define GRID_WIDTH 35
#define GRID_HEIGHT 9

Grid::Grid()
: Scene("grid")
{
}

Grid::~Grid()
{
}

void Grid::setup()
{
    Scene::setup();
    
    // params
    mDrawDynamicTexture = true;

	// load texture
	try { mTexture = gl::Texture( loadImage( loadResource("earthDiffuse.png") ) ); }
	catch( const std::exception &e ) { console() << e.what() << std::endl; }

    setupDynamicTexture();
}

void Grid::reset()
{
    
}

void Grid::setupDynamicTexture()
{
    // VTF
    // Initialize FBO
	gl::Fbo::Format format;
	format.setColorInternalFormat( GL_RGB32F_ARB );
	mVtfFbo = gl::Fbo( GRID_WIDTH*5, GRID_HEIGHT*5, format );
    
	// Initialize FBO texture
	mVtfFbo.bindFramebuffer();
	gl::setViewport( mVtfFbo.getBounds() );
	gl::clear();
	mVtfFbo.unbindFramebuffer();
	mVtfFbo.getTexture().setWrap( GL_REPEAT, GL_REPEAT );
    
    // Params
    mNoiseScale         = Vec3f(1.0f,1.0f,0.25f);
    mDisplacementHeight	= 8.0f;
	mDisplacementSpeed  = 1.0f;
	mTheta				= 0.0f;
    
    // Load shaders
	try {
		mShaderTex = gl::GlslProg( loadResource( RES_SHADER_CT_TEX_VERT ), loadResource( RES_SHADER_CT_TEX_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << "Unable to compile texture shader:\n" << ex.what() << "\n";
	}
	try {
		//mShaderFractal = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_SIMPLICITY_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << "Unable to compile VTF shader:\n" << ex.what() << "\n";
	}
    
    try {
		mShaderPixelate = gl::GlslProg( loadResource( RES_PASSTHRU_VERT ), loadResource( RES_SHADER_PIXELATE_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << "Unable to compile pixelate shader:\n" << ex.what() << "\n";
	}
    
//    int levels = 32;
//    int stripSize = levels * 2;
//    mStripFbo = gl::Fbo( stripSize, 1, format );
//    
//    bool border = false;
//    Surface32f stripSurface( mStripFbo.getTexture() );
//	Surface32f::Iter it = stripSurface.getIter();
//	while( it.line() ){
//		while( it.pixel() ){
//			it.r() = border ? 0.0f : 1.0f;
//            it.g() = 0.0f;
//            it.b() = 0.0f;
//            it.a() = 1.0f;
//            border = !border;
//		}
//	}
//	
//	gl::Texture stripTexture( stripSurface );
//	mStripFbo.bindFramebuffer();
//	gl::setMatricesWindow( mStripFbo.getSize(), false );
//	gl::setViewport( mStripFbo.getBounds() );
//	gl::draw( stripTexture );
//	mStripFbo.unbindFramebuffer();
//	mStripFbo.getTexture().setWrap( GL_REPEAT, GL_REPEAT );
}

void Grid::setupInterface()
{
    mInterface->addParam(CreateFloatParam( "disp_speed", &mDisplacementSpeed )
                         .maxValue(3.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "disp_height", &mDisplacementHeight )
                         .maxValue(20.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateVec3fParam("noise", &mNoiseScale, Vec3f::zero(), Vec3f(50.0f,50.0f,50.0f))
                         .oscReceiver(mName));
    
}

void Grid::setupDebugInterface()
{
    Scene::setupDebugInterface(); // add all interface params
}

void Grid::update(double dt)
{
    Scene::update(dt);
    
    gl::disableAlphaBlending();
	gl::disableDepthRead();
	gl::disableDepthWrite();
	gl::color( Color( 1, 1, 1 ) );
    
    // update noise
	float time = (float)getElapsedSeconds() * mDisplacementSpeed;
	mTheta = time;//math<float>::sin( time );
    
    if (mDrawDynamicTexture)
    {
        drawDynamicTexture(); // always draw to fbo from update, not draw
    }
}

void Grid::draw()
{
	gl::enableAlphaBlending();

    gl::disableDepthWrite();
	gl::disableDepthRead();

	gl::pushMatrices();
    gl::setMatricesWindow( mApp->getViewportWidth(), mApp->getViewportHeight() );

    if (mDrawDynamicTexture)
    {
        drawFromDynamicTexture();
    }
    else
    {
        drawPixels();
    }

	// restore camera and render states
    gl::disable( GL_TEXTURE_2D );
	gl::popMatrices();

	gl::disableDepthWrite();
	gl::disableDepthRead();

	gl::disableAlphaBlending();
}

void Grid::drawDynamicTexture()
{
    // Bind FBO and set up window
    mVtfFbo.bindFramebuffer();
    gl::setViewport( mVtfFbo.getBounds() );
    gl::setMatricesWindow( mVtfFbo.getSize() );
    gl::clear();
    
    //mStripFbo.bindTexture(1);
     
    // Bind and configure dynamic texture shader
    mShaderTex.bind();
    mShaderTex.uniform( "theta", mTheta );
    mShaderTex.uniform( "scale", mNoiseScale );
    //mShaderTex.uniform( "tex", 1 );
    //    mShaderTex.uniform( "u_time", mTheta );
    //    mShaderTex.uniform( "u_scale", 1.0f );
    //    mShaderTex.uniform( "u_RenderSize", mVtfFbo.getSize() );
    
    // Draw shader output
    gl::enable( GL_TEXTURE_2D );
    gl::color( Colorf::white() );
    gl::begin( GL_TRIANGLES );
    
    // Define quad vertices
    Vec2f vert0( (float)mVtfFbo.getBounds().x1, (float)mVtfFbo.getBounds().y1 );
    Vec2f vert1( (float)mVtfFbo.getBounds().x2, (float)mVtfFbo.getBounds().y1 );
    Vec2f vert2( (float)mVtfFbo.getBounds().x1, (float)mVtfFbo.getBounds().y2 );
    Vec2f vert3( (float)mVtfFbo.getBounds().x2, (float)mVtfFbo.getBounds().y2 );
    
    // Define quad texture coordinates
    Vec2f uv0( 0.0f, 0.0f );
    Vec2f uv1( 1.0f, 0.0f );
    Vec2f uv2( 0.0f, 1.0f );
    Vec2f uv3( 1.0f, 1.0f );
    
    // Draw quad (two triangles)
    gl::texCoord( uv0 );
    gl::vertex( vert0 );
    gl::texCoord( uv2 );
    gl::vertex( vert2 );
    gl::texCoord( uv1 );
    gl::vertex( vert1 );
    
    gl::texCoord( uv1 );
    gl::vertex( vert1 );
    gl::texCoord( uv2 );
    gl::vertex( vert2 );
    gl::texCoord( uv3 );
    gl::vertex( vert3 );
    
    gl::end();
    gl::disable( GL_TEXTURE_2D );
    
    // Unbind everything
    mShaderTex.unbind();
    mVtfFbo.unbindFramebuffer();
    
    ///////////////////////////////////////////////////////////////
}

void Grid::drawFromDynamicTexture()
{
    gl::enable( GL_TEXTURE_2D );
    gl::color( Colorf::white() );
    gl::setMatricesWindow( mApp->getViewportSize() );
    
    mVtfFbo.getTexture().bind(1);
    //mTexture.bind(1);
    
    const float windowWidth = mApp->getViewportWidth();
    const float windowHeight = mApp->getViewportHeight();
    
    float pixWidth = windowWidth / GRID_WIDTH;
    float pixHeight = windowHeight / GRID_HEIGHT;
    
    mShaderPixelate.bind();
    mShaderPixelate.uniform( "tex", 1 );
    mShaderPixelate.uniform( "pixelWidth", pixWidth );
    mShaderPixelate.uniform( "pixelHeight", pixHeight );
    
    mShaderPixelate.uniform( "imageWidth", windowWidth );
    mShaderPixelate.uniform( "imageHeight", windowHeight );
    
    Rectf preview( 0.0f, 0.0f, windowWidth, windowHeight );
    gl::drawSolidRect( preview );

    mShaderPixelate.unbind();
    mVtfFbo.getTexture().unbind();
    //mTexture.unbind();
}

void Grid::drawPixels()
{
    const float windowWidth = mApp->getViewportWidth();
    const float windowHeight = mApp->getViewportHeight();
    
    float pixWidth = windowWidth / GRID_WIDTH;
    float pixHeight = windowHeight / GRID_HEIGHT;
    
    gl::setMatricesWindow( mApp->getViewportSize() );
    
    Surface32f testSurface( mTexture );
    
    // grid
    gl::disable( GL_TEXTURE_2D );
    gl::color(1.0f,1.0f,1.0f,1.0f);
    float x = 0;
    float y = 0;
    //int pixWidth = mApp->getWindowWidth() / GRID_WIDTH;
    //int pixHeight = mApp->getWindowHeight() / GRID_HEIGHT;
    float imageWidth = testSurface.getWidth();
    float imageHeight = testSurface.getHeight();
    float dx = pixWidth / imageWidth;
    //float dy = pixHeight / imageHeight;
    
    bool drawGrid = false;
    
    for (int i = 0; i < GRID_HEIGHT; ++i)
    {
        x = 0;
        for (int j = 0; j < GRID_WIDTH; ++j)
        {
            gl::color( testSurface.getPixel(Vec2i((x/windowWidth)*imageWidth,(y/mApp->getWindowHeight())*imageHeight)) );
            drawSolidRect( Rectf( x, y, x+pixWidth, y+pixHeight ) );
            if (drawGrid)
            {
                gl::color( Colorf::white() );
                drawStrokedRect( Rectf( x, y, x+pixWidth, y+pixHeight ) );
            }
            x += pixWidth;
        }
        y += pixHeight;
    }
}

void Grid::drawDebug()
{
    gl::enable( GL_TEXTURE_2D );
    gl::setMatricesWindow( getWindowSize() );
    
    Rectf preview( 200.0f, mApp->getWindowHeight() - mVtfFbo.getHeight(), 200.0f + mVtfFbo.getWidth(), mApp->getWindowHeight() );
    gl::draw( mVtfFbo.getTexture(), mVtfFbo.getBounds(), preview );
    
    // grid
    gl::setMatricesWindow( mApp->getViewportSize() );
    gl::disable( GL_TEXTURE_2D );
    gl::color(1.0f,1.0f,1.0f,1.0f);
    int x = 0;
    int y = 0;
    int pixWidth = mApp->getViewportWidth() / GRID_WIDTH;
    int pixHeight = mApp->getViewportHeight() / GRID_HEIGHT;
   
    for (int i = 0; i < GRID_HEIGHT; ++i)
    {
        x = 0;
        for (int j = 0; j < GRID_WIDTH; ++j)
        {
            drawStrokedRect( Rectf( x, y, x+pixWidth, y+pixHeight ) );
            x += pixWidth;
        }
        y += pixHeight;
    }
}
