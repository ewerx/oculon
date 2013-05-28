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
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::gl;
using namespace ci::app;
using namespace std;


Grid::Grid()
: Scene("grid")
{
    mAudioInputHandler.setup(this, false);
}

Grid::~Grid()
{
}

void Grid::setup()
{
    Scene::setup();
    
    // params
    mShaderType             = SHADER_NOISE;
    mGridMode               = GRIDMODE_PIXELS;
    mMotionBlurEnabled      = false;
    
    mColorScheme            = COLORSCHEME_REDFIRE;
    setColorScheme();
    mLowPassSplit           = 0.1f;
    mHighPassSplit          = 0.5f;
    
    mGroupCols              = 0;
    mGroupRows              = 0;
    

//	// load texture
	try { mTexture = gl::Texture( loadImage( loadResource("earthDiffuse.png") ) ); }
	catch( const std::exception &e ) { console() << e.what() << std::endl; }

    setupDynamicTexture();
    
    mMotionBlurRenderer.setup( mApp->getViewportSize(), boost::bind( &Grid::drawPixels, this ) );
    
    reset();
}

void Grid::reset()
{
    for (int i=0; i < GRID_WIDTH; ++i)
    {
        for (int j=0; j < GRID_HEIGHT; ++j)
        {
            mPixels[i][j].mValue = 0.0f;
        }
    }
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
		mShaders[SHADER_NOISE] = gl::GlslProg( loadResource( RES_SHADER_CT_TEX_VERT ), loadResource( RES_SHADER_CT_TEX_FRAG ) );
        
        mShaderPixelate = gl::GlslProg( loadResource( RES_PASSTHRU_VERT ), loadResource( RES_SHADER_PIXELATE_FRAG ) );
        
        
        
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << "Unable to compile texture shader:\n" << ex.what() << "\n";
	} catch ( ... ) {
		console() << "Unable to load shader" << std::endl;
	}
}

void Grid::setupInterface()
{
    vector<string> modeNames;
#define GRIDMODE_ENTRY( nam, enm ) \
modeNames.push_back(nam);
    GRIDMODE_TUPLE
#undef  GRIDMODE_ENTRY
    mInterface->addEnum(CreateEnumParam( "Mode", (int*)(&mGridMode) )
                        .maxValue(GRIDMODE_COUNT)
                        .oscReceiver(getName(), "gridmode")
                        .isVertical(), modeNames);
    
    mInterface->addParam(CreateIntParam( "group_cols", &mGroupCols )
                         .maxValue(GRID_WIDTH/2)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateIntParam( "group_rows", &mGroupRows )
                         .maxValue(GRID_HEIGHT/2)
                         .oscReceiver(getName()));
    
    mInterface->addParam(CreateBoolParam( "motion_blur", &mMotionBlurEnabled )
                         .oscReceiver(getName()));
    
    
    
    // colors
    mInterface->gui()->addColumn();
    vector<string> colorSchemeNames;
#define COLORSCHEME_ENTRY( nam, enm ) \
colorSchemeNames.push_back(nam);
    COLORSCHEME_TUPLE
#undef  COLORSCHEME_ENTRY
    mInterface->addEnum(CreateEnumParam( "Color Scheme", (int*)(&mColorScheme) )
                        .maxValue(COLORSCHEME_COUNT)
                        .oscReceiver(getName(), "colorscheme")
                        .isVertical(), colorSchemeNames)->registerCallback( this, &Grid::setColorScheme );
    mInterface->addParam(CreateFloatParam( "lowpass", &mLowPassSplit )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "highpass", &mHighPassSplit )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateColorParam("Color 1", &mColor1, kMinColor, kMaxColor)
                        .oscReceiver(mName,"color1"));
    mInterface->addParam(CreateColorParam("Color 2", &mColor2, kMinColor, kMaxColor)
                        .oscReceiver(mName,"color2"));
    mInterface->addParam(CreateColorParam("Color 3", &mColor3, kMinColor, kMaxColor)
                        .oscReceiver(mName,"color3"));
    
    // shader
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("Perlin Shader");
    mInterface->addParam(CreateFloatParam( "disp_speed", &mDisplacementSpeed )
                         .maxValue(3.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateVec3fParam("noise", &mNoiseScale, Vec3f::zero(), Vec3f(50.0f,50.0f,5.0f))
                         .oscReceiver(mName));
    
    
    mAudioInputHandler.setupInterface( mInterface );
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
    
    if (mGridMode == GRIDMODE_SHADER)
    {
        drawDynamicTexture(); // always draw to fbo from update, not draw
    }
    else
    {
        mAudioInputHandler.update(dt, mApp->getAudioInput());
        
        gl::enableAlphaBlending();
        if (mMotionBlurEnabled)
        {
            mMotionBlurRenderer.preDraw();
        }
    }
}

void Grid::draw()
{
	gl::enableAlphaBlending();

    gl::disableDepthWrite();
	gl::disableDepthRead();

	gl::pushMatrices();
    gl::setMatricesWindow( mApp->getViewportWidth(), mApp->getViewportHeight() );

    if (mGridMode == GRIDMODE_SHADER)
    {
        drawFromDynamicTexture();
    }
    else
    {
        if (mMotionBlurEnabled)
        {
            mMotionBlurRenderer.preDraw();
        }
        else
        {
            drawPixels();
        }
    }

	// restore camera and render states
    gl::disable( GL_TEXTURE_2D );
	gl::popMatrices();

	gl::disableDepthWrite();
	gl::disableDepthRead();

	gl::disableAlphaBlending();
}

void Grid::shaderPreDraw()
{
    // Bind and configure dynamic texture shader
    mShaders[mShaderType].bind();
    
    switch (mShaderType)
    {
        case SHADER_NOISE:
            mShaders[mShaderType].uniform( "theta", mTheta );
            mShaders[mShaderType].uniform( "scale", mNoiseScale );
            break;
            
        default:
            break;
    }
    
}

void Grid::shaderPostDraw()
{
    mShaders[mShaderType].unbind();
}

void Grid::drawDynamicTexture()
{
    // Bind FBO and set up window
    mVtfFbo.bindFramebuffer();
    gl::setViewport( mVtfFbo.getBounds() );
    gl::setMatricesWindow( mVtfFbo.getSize() );
    gl::clear();
     
    shaderPreDraw();
    
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
    shaderPostDraw();
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

#pragma mark - Pixels

void Grid::drawPixels()
{
    // dimensions
    const float windowWidth = mApp->getViewportWidth();
    const float windowHeight = mApp->getViewportHeight();
    
    const float pixWidth = windowWidth / GRID_WIDTH;
    const float pixHeight = windowHeight / GRID_HEIGHT;
    
    // draw grid
    gl::setMatricesWindow( mApp->getViewportSize() );
    gl::disable( GL_TEXTURE_2D );
    gl::color( ColorAf::white() );
    float x = 0;
    float y = 0;
    
    int index = 0;
    
    AudioInputHandler::FftValues::const_iterator audioIt = mAudioInputHandler.fftValuesBegin();
    //AudioInputHandler::FftValues& fftValues = mAudioInputHandler.getFftValues();
    
    for (int row = 0; row < GRID_HEIGHT; ++row)
    {
        x = 0;
        for (int col = 0; col < GRID_WIDTH; ++col)
        {
            if (audioIt == mAudioInputHandler.fftValuesEnd())
            {
                audioIt = mAudioInputHandler.fftValuesBegin();
            }
            
            const AudioInputHandler::tFftValue fftValue = (*audioIt);
            
            float value = fftValue.mValue;
            
            if (row > 0 && mGroupRows > 1 && ((row+0) % mGroupRows != 0))
            {
                value = mPixels[row-1][col].mValue;
            }
            
            mPixels[row][col].mValue = value;
            float freq = (float)(fftValue.mBandIndex) / (float)(mApp->getAudioInput().getFft()->getBinSize());
            //mApp->console() << fftValue.mBandIndex << "\t(" << freq << ")\t" << value << std::endl;
            if (freq < mLowPassSplit) {
                float m = 1.0f - freq/mLowPassSplit;
                gl::color( mColor1.r * m, mColor1.g * m, mColor1.b * m, value );
            } else if (mHighPassSplit > mLowPassSplit && freq < mHighPassSplit) {
                float m = freq/(mHighPassSplit-mLowPassSplit);
                gl::color( mColor2.r * m, mColor2.g * m, mColor2.b * m, value );
            } else {
                float m = (1.0f - freq)/(1.0f - mHighPassSplit);
                gl::color( mColor3.r * m, mColor3.g * m, mColor3.b * m, value );
            }
            drawSolidRect( Rectf( x, y, x+pixWidth, y+pixHeight ) );
            
            // draw grid
//            gl::color( Colorf::white() );
//            drawStrokedRect( Rectf( x, y, x+pixWidth, y+pixHeight ) );
            
            x += pixWidth;
            if (mGroupCols == 0 || ((col+1) % mGroupCols == 0))
            {
                audioIt++;
            }
            ++index;
        }
        if (mGroupCols > 0)
        {
            audioIt++;
        }
        y += pixHeight;
    }
}

#pragma mark -

bool Grid::setColorScheme()
{
    switch (mColorScheme)
    {
        case COLORSCHEME_REDFIRE:
            mColor1 = ColorAf( 0.92f, 0.0f, 0.227f, 1.0f );
            mColor2 = ColorAf( 1.0f, 0.372f, 0.0f, 1.0f );
            mColor3 = ColorAf( 1.0f, 0.643f, 0.0f, 1.0f );
            break;
            
        case COLORSCHEME_BLUEFIRE:
            break;
            
        case COLORSCHEME_ICE:
            break;
            
        case COLORSCHEME_GREEN:
            break;
            
        default:
            break;
    }
}