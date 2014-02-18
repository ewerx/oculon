//
//  ParsecLines.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2/14/2014.
//
//

#include "ParsecLines.h"
#include "cinder/Rand.h"

using namespace std;
using namespace ci;
using namespace ci::app;

ParsecLines::ParsecLines()
{
}

ParsecLines::~ParsecLines()
{
}

void ParsecLines::setup(vector<ParsecStars::Star>& stars)
{
    mFboSize = 128;
    mNumParticles = mFboSize * mFboSize;
    
    // shaders
    mRenderShader = gl::GlslProg( loadResource( "parsec_lines_vert.glsl" ), loadResource( "parsec_lines_frag.glsl" ) );
    mSimulationShader = gl::GlslProg( loadResource( "parsec_simulation_vert.glsl" ), loadResource( "parsec_simulation_frag.glsl" ));//gl::GlslProg( loadResource( "lines_simulation_vert.glsl" ), loadResource( "lines_simulation_frag.glsl" ));
    
    // setup fbo
    Surface32f posSurface = Surface32f(mFboSize,mFboSize,true);
	Surface32f velSurface = Surface32f(mFboSize,mFboSize,true);
	Surface32f infoSurface = Surface32f(mFboSize,mFboSize,true);
    
	Surface32f::Iter iterator = posSurface.getIter();
    int index = 0;
    
    while(iterator.line())
	{
		while(iterator.pixel())
		{
            if (index >= stars.size()) {
                assert(false);
                break;
            }
            
            ParsecStars::Star& star = stars[index];
            Vec3f pos = Rand::randVec3f();//star.getPosition();
            float magnitude = star.getMagnitude();
            posSurface.setPixel(iterator.getPos(), ColorA(pos.x,pos.y,pos.z,magnitude));
            ++index;
            
			velSurface.setPixel(iterator.getPos(), ColorA(0.0f,0.0f,0.0f,0.0f));
            infoSurface.setPixel(iterator.getPos(), ColorA(0.0f,0.0f,0.0f,0.0f));
		}
	}
    
    std::vector<Surface32f> surfaces;
    surfaces.push_back( posSurface );
    surfaces.push_back( velSurface );
    surfaces.push_back( infoSurface );
    mParticlesFbo = PingPongFbo( surfaces );
    
    gl::Texture::Format format;
    format.setInternalFormat( GL_RGBA32F_ARB );
	
	mInitialPosTex = gl::Texture(posSurface, format);
	mInitialPosTex.setWrap( GL_REPEAT, GL_REPEAT );
	mInitialPosTex.setMinFilter( GL_NEAREST );
	mInitialPosTex.setMagFilter( GL_NEAREST );
	
	mInitialVelTex = gl::Texture(velSurface, format);
	mInitialVelTex.setWrap( GL_REPEAT, GL_REPEAT );
	mInitialVelTex.setMinFilter( GL_NEAREST );
	mInitialVelTex.setMagFilter( GL_NEAREST );
    
    // setup VBO
    // A dummy VboMesh the same size as the texture to keep the vertices on the GPU
    vector<Vec2f> texCoords;
    vector<uint32_t> indices;
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    layout.setStaticTexCoords2d();
    layout.setStaticNormals();
    
    mVboMesh = gl::VboMesh( mNumParticles, mNumParticles, layout, GL_LINES);
    for( int y = 0; y < mFboSize; ++y ) {
        for( int x = 0; x < mFboSize; ++x ) {
            indices.push_back( y * mFboSize + x );
            texCoords.push_back( Vec2f( x/(float)mFboSize, y/(float)mFboSize ) );
        }
    }
    mVboMesh.bufferIndices( indices );
    mVboMesh.bufferTexCoords2d( 0, texCoords );
    mVboMesh.unbindBuffers();
}

void ParsecLines::update()
{
    // simulation - update position/vel textures
    gl::pushMatrices();
    gl::setMatricesWindow( mParticlesFbo.getSize(), false ); // false to prevent vertical flipping
    gl::setViewport( mParticlesFbo.getBounds() );
    mParticlesFbo.bindUpdate();
    
    mSimulationShader.bind();
    mSimulationShader.uniform( "positions", 0 );
    mSimulationShader.uniform( "velocities", 1 );
    mSimulationShader.uniform( "information", 2);
    gl::drawSolidRect(mParticlesFbo.getBounds());
    mSimulationShader.unbind();
    
    mParticlesFbo.unbindUpdate();
    gl::popMatrices();
}

void ParsecLines::draw()
{
	if(!(mRenderShader && mVboMesh)) return;
    
	preRender();
    
	gl::color( Color::white() );
	gl::draw( mVboMesh );
    
	postRender();
}

void ParsecLines::preRender()
{
	// store current OpenGL state
	glPushAttrib( GL_LINE_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT );
    
    gl::enableAdditiveBlending();
    gl::enableDepthRead();
    gl::enableDepthWrite();
//
//    glEnable(GL_TEXTURE_2D);
    
    gl::lineWidth(1.0f);
    
    // bind textures
    mParticlesFbo.bindTexture(0);//pos
    mParticlesFbo.bindTexture(1);//vel
    mParticlesFbo.bindTexture(2);//info
    //TODO: bind audio
    
	// bind shader
	mRenderShader.bind();
    mRenderShader.uniform("posMap", 0);
    mRenderShader.uniform("velMap", 1);
    mRenderShader.uniform("information", 2);
//	mRenderShader.uniform("tex0", 3);
//	mRenderShader.uniform("tex1", 4);
//	mRenderShader.uniform("time", (float) getElapsedSeconds() );
}

void ParsecLines::postRender()
{
	// unbind shader
	mRenderShader.unbind();
    
    // unbind textures
    mParticlesFbo.unbindTexture();
	
	// restore OpenGL state
	glPopAttrib();
}
