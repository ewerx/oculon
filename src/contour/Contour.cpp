//
//  Contour.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2012-12-28.
//
//


#include "Contour.h"
#include "Interface.h"
#include "OculonApp.h"
#include "Resources.h"
#include "cinder/Camera.h"
#include "cinder/ImageIo.h"
#include "cinder/app/AppBasic.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define VTF_FBO_SIZE 640

Contour::Contour()
: Scene("contour")
{
}

Contour::~Contour()
{
}

void Contour::setup()
{
    Scene::setup();

	// load texture
//	try { mTexture = gl::Texture( loadImage( loadResource("spectrum.png") ) ); }
//	catch( const std::exception &e ) { console() << e.what() << std::endl; }

	mZoom = Vec2f( 13.6, 8 );

	//openFile();
    setupDynamicTexture();
}

void Contour::setupDynamicTexture()
{
    // VTF
    // Initialize FBO
	gl::Fbo::Format format;
	format.setColorInternalFormat( GL_RGB32F_ARB );
	mVtfFbo = gl::Fbo( VTF_FBO_SIZE, VTF_FBO_SIZE, format );
    
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
		mShaderVtf = gl::GlslProg( loadResource( RES_SHADER_VTF_VERT ), loadResource( RES_SHADER_VTF_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << "Unable to compile VTF shader:\n" << ex.what() << "\n";
	}
    
    int levels = 32;
    int stripSize = levels * 2;
    mStripFbo = gl::Fbo( stripSize, 1, format );
    
    bool border = false;
    Surface32f stripSurface( mStripFbo.getTexture() );
	Surface32f::Iter it = stripSurface.getIter();
	while( it.line() ){
		while( it.pixel() ){
			it.r() = border ? 0.0f : 1.0f;
            it.g() = 0.0f;
            it.b() = 0.0f;
            it.a() = 1.0f;
            border = !border;
		}
	}
	
	gl::Texture stripTexture( stripSurface );
	mStripFbo.bindFramebuffer();
	gl::setMatricesWindow( mStripFbo.getSize(), false );
	gl::setViewport( mStripFbo.getBounds() );
	gl::draw( stripTexture );
	mStripFbo.unbindFramebuffer();
	mStripFbo.getTexture().setWrap( GL_REPEAT, GL_REPEAT );
}

void Contour::openFile()
{
	fs::path path = getOpenFilePath( "", ImageIo::getLoadExtensions() );
	if( ! path.empty() ) {
		Surface32f image = loadImage( path );
        
		SurfaceRaster raster = SurfaceRaster(image);
        
        mMinValue = numeric_limits<float>::max();
        mMaxValue = numeric_limits<float>::min();
        
        Surface32f::Iter pixelIter = image.getIter();
        while( pixelIter.line() ) {
            while( pixelIter.pixel() ) {
                mMinValue = math<float>::min( mMinValue, pixelIter.r() );
                mMaxValue = math<float>::max( mMaxValue, pixelIter.r() );
            }
        }
        
        if(mMaxValue==mMinValue)
        {
            mMaxValue += 0.1f;
            mMinValue -= 0.1f;
        }
        
        if(mMaxValue-mMinValue < 1e-10)
        {
            double vdiff = 1e-10;
            double vcenter = (mMaxValue-mMinValue)*0.5f;
            mMaxValue = vcenter + vdiff*0.5f;
            mMinValue = vcenter - vdiff*0.5f;
        }
        
        mLevels = 50;
        mContourMap.generate_levels(mMinValue,mMaxValue,mLevels);
        mContourMap.contour(&raster);
        //mContourMap.dump();
        mContourMap.consolidate();
        mContourMap.dump();
        
        
	}
}

void Contour::setupInterface()
{
    mInterface->addParam(CreateVec2fParam("zoom", &mZoom, Vec2f::zero(), Vec2f(100.0f,100.0f))
                         .oscReceiver(getName())
                         .isXYPad());
    
    mInterface->addParam(CreateFloatParam( "disp_speed", &mDisplacementSpeed )
                         .maxValue(3.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "disp_height", &mDisplacementHeight )
                         .maxValue(20.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateVec3fParam("noise", &mNoiseScale, Vec3f::zero(), Vec3f(50.0f,50.0f,50.0f))
                         .oscReceiver(getName()));
    
}

//void Contour::setupDebugInterface()
//{
//    Scene::setupDebugInterface(); // add all interface params
//}

void Contour::update(double dt)
{
    Scene::update(dt);
    
    gl::disableAlphaBlending();
	gl::disableDepthRead();
	gl::disableDepthWrite();
	gl::color( Color( 1, 1, 1 ) );
    
    // update noise
	float time = (float)getElapsedSeconds() * mDisplacementSpeed;
	mTheta = time;//math<float>::sin( time );
    drawDynamicTexture(); // always draw to fbo from update, not draw
}

void Contour::draw()
{
	gl::enableAlphaBlending();

	//gl::enableDepthRead();
	//gl::enableDepthWrite();
    gl::disableDepthWrite();
	gl::disableDepthRead();
    
    //gl::enable( GL_TEXTURE_2D );
    gl::disable( GL_TEXTURE_2D );
	gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
    
    //CameraPersp cam( getWindowWidth(), getWindowHeight(), 60.0f, 0.1f, 500.0f );
    //cam.setEyePoint( Vec3f(0, 0, -100.0f) );
    //cam.setCenterOfInterestPoint( Vec3f::zero() );

	// enable 3D camera
	gl::pushMatrices();
	//gl::setMatrices( getCamera() );
    gl::setMatricesWindow( mApp->getViewportWidth(), mApp->getViewportHeight() );

	//drawContourLines();
    drawFromDynamicTexture();

	// restore camera and render states
    gl::disable( GL_TEXTURE_2D );
	gl::popMatrices();

	gl::disableDepthWrite();
	gl::disableDepthRead();

	gl::disableAlphaBlending();
}

void Contour::drawDynamicTexture()
{
    // Bind FBO and set up window
    mVtfFbo.bindFramebuffer();
    gl::setViewport( mVtfFbo.getBounds() );
    gl::setMatricesWindow( mVtfFbo.getSize() );
    gl::clear();
    
    mStripFbo.bindTexture(1);
     
    // Bind and configure dynamic texture shader
    mShaderTex.bind();
    mShaderTex.uniform( "theta", mTheta );
    mShaderTex.uniform( "scale", mNoiseScale );
    mShaderTex.uniform( "tex", 1 );
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

void Contour::drawContourLines()
{
    int width = mApp->getViewportWidth();
    int height = mApp->getViewportHeight();
    
    vector<Path2d> paths;
    vector<double> altitudes;
    
    for( int i=0; i < mLevels; ++i )
    {
        CContourLevel *level = mContourMap.level(i);
        if(!level || !level->contour_lines) continue;
        for(int j=0; j<level->contour_lines->size(); j++)
        {
            CContour *line = level->contour_lines->at(j);
            vector<SPoint> points = line->contourPoints();
            Vec2f oldPoint;
            int emptyCount = 0;
            Path2d path;
            for(int p=0; p<points.size(); p++)
            {
                Vec2f point(points[p].x * mZoom.x, points[p].y * mZoom.y);
                if(p)
                {
                    path.lineTo(point);
                    Vec2f diff = point - oldPoint;
                    if(diff.x == 0 || diff.y == 0) emptyCount++;
                }
                else path.moveTo(point);
                if(emptyCount > points.size()*0.2f) break;
                oldPoint = point;
            }
            if(emptyCount / (float) points.size() > 0.2) continue; // we dont want stuff that has loads of empty segments
            paths.push_back(path);
        }
        altitudes.push_back(mContourMap.alt(i));
    }
    
    for(int i=0; i < math<int>::min(paths.size(),300); i++)
    {
        gl::draw(paths.at(i));
        //painter.setPen(QPen(plotColor, 0.25 + i/(double)paths.size()*plotThickness, style));
        //painter.drawPath(paths.at(i));
        //qDebug() << "path length:" << paths.at(i).length();
    }
}

void Contour::drawFromDynamicTexture()
{
    gl::enable( GL_TEXTURE_2D );
    gl::setMatricesWindow( getWindowSize() );
    
    Rectf preview( 0.0f, 0.0f, mApp->getWindowWidth(), mApp->getWindowHeight() );
    gl::draw( mVtfFbo.getTexture(), mVtfFbo.getBounds(), preview );
}

void Contour::drawDebug()
{
    gl::enable( GL_TEXTURE_2D );
    gl::setMatricesWindow( getWindowSize() );
    
    Rectf preview( 100.0f, mApp->getWindowHeight() - 200.0f, 180.0f, mApp->getWindowHeight() - 120.0f );
    gl::draw( mVtfFbo.getTexture(), mVtfFbo.getBounds(), preview );
}

const Camera& Contour::getCamera()
{    
    return mApp->getMayaCam();
}
