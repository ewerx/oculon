//
//  Dust.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 11/21/2013.
//
//

#include "Dust.h"
#include "OculonApp.h"
#include "Interface.h"
#include "cinder/Perlin.h"
#include "cinder/Rand.h"

#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#pragma mark - Construction

Dust::Dust()
: Scene("dust")
{
}

Dust::~Dust()
{
}

#pragma mark - Setup

void Dust::setup()
{
    Scene::setup();
    
    mSimulationShader = loadVertAndFragShaders("dust_simulation_vert.glsl", "dust_simulation_frag.glsl");
    mRenderShader = loadVertAndFragShaders("dust_render_vert.glsl", "dust_render_frag.glsl");
    
    mSpriteTex = gl::Texture( loadImage( app::loadResource( "glitter.png" ) ) );
    
    setupFBO();
    setupVBO();
    
    mAudioInputHandler.setup(false);
    
    mReset = false;
    mAudioTime = false;
    
    // params
    mTimeStep = 0.1f;
    mPointSize = 1.0f;
    mDecayRate = 0.5f;
    
    mAudioReactive = false;
    mUseDynamicTex = true;
    setupDynamicTexture();
}

void Dust::setupDynamicTexture()
{
    mNoiseTheta = 0.0f;
    mNoiseSpeed = 0.1f;
    mNoiseScale = Vec3f(1.0f,1.0f,0.25f);
    
    mDynamicTexShader = loadFragShader("simplex_frag.glsl");
    
    gl::Fbo::Format format;
	format.setColorInternalFormat( GL_RGB32F_ARB );
	mDynamicTexFbo = gl::Fbo( kBufSize, kBufSize, format );
    
    // initialize
    mDynamicTexFbo.bindFramebuffer();
	gl::setViewport( mDynamicTexFbo.getBounds() );
	gl::clear();
	mDynamicTexFbo.unbindFramebuffer();
	mDynamicTexFbo.getTexture().setWrap( GL_REPEAT, GL_REPEAT );
}

void Dust::setupFBO()
{
    console() << "[dust] initializing " << kNumParticles << " particles, hang on!" << std::endl;
    //initialize buffer
	Surface32f posSurface = Surface32f(kBufSize,kBufSize,true);
	Surface32f velSurface = Surface32f(kBufSize,kBufSize,true);
	Surface32f infoSurface = Surface32f(kBufSize,kBufSize,true);
    Surface32f noiseSurface = Surface32f(kBufSize,kBufSize,true);
    
	Surface32f::Iter iterator = posSurface.getIter();
	
    Perlin perlin(32, clock() * .1f);
    
	while(iterator.line())
	{
		while(iterator.pixel())
		{
            float x = Rand::randFloat(mApp->getViewportWidth()) / (float)mApp->getViewportWidth();
            float y = Rand::randFloat(mApp->getViewportHeight()) / (float)mApp->getViewportHeight();
            float z = 0.0f;
            float mass = Rand::randFloat(0.01f,1.0f);
            
            // position + mass
			posSurface.setPixel(iterator.getPos(), ColorA(x,y,z,mass));
            
            // velocity
            float vx = Rand::randFloat(-.005f,.005f);
            float vy = Rand::randFloat(-.005f,.005f);
            float vz = 1.0f;
            float age = Rand::randFloat(.007f,0.9f);
            
			// velocity + age
			velSurface.setPixel(iterator.getPos(), ColorA(vx,vy,vz,age));
            
			// decay + max age
            float decay = Rand::randFloat(.01f,10.00f);
            float maxAge = Rand::randFloat(1.0f,10.0f);
			infoSurface.setPixel(iterator.getPos(),
                                 ColorA(decay, maxAge, 0.0f, 0.0f));
            
            // noise
            float nX = iterator.x() * 0.005f;
            float nY = iterator.y() * 0.005f;
            float nZ = app::getElapsedSeconds() * 0.1f;
            Vec3f v( nX, nY, nZ );
            float noise = perlin.fBm( v );
            
            float angle = noise * 15.0f;
            
            noiseSurface.setPixel(iterator.getPos(),
                                  ColorA( cos( angle ) * Rand::randFloat(.1f,.4f), sin( angle ) * Rand::randFloat(.1f,.4f), 0.25f, 1.0f ));
		}
	}
    
    std::vector<Surface32f> surfaces;
    surfaces.push_back( posSurface );
    surfaces.push_back( velSurface );
    mParticlesFbo = PingPongFbo( surfaces );
    
    gl::Texture::Format format;
    format.setInternalFormat( GL_RGBA32F_ARB );
    
    mNoiseTex = gl::Texture(noiseSurface, format);
	mNoiseTex.setWrap( GL_REPEAT, GL_REPEAT );
	mNoiseTex.setMinFilter( GL_NEAREST );
	mNoiseTex.setMagFilter( GL_NEAREST );
	
	mInitialPosTex = gl::Texture(posSurface, format);
	mInitialPosTex.setWrap( GL_REPEAT, GL_REPEAT );
	mInitialPosTex.setMinFilter( GL_NEAREST );
	mInitialPosTex.setMagFilter( GL_NEAREST );
	
	mInitialVelTex = gl::Texture(velSurface, format);
	mInitialVelTex.setWrap( GL_REPEAT, GL_REPEAT );
	mInitialVelTex.setMinFilter( GL_NEAREST );
	mInitialVelTex.setMagFilter( GL_NEAREST );
    
	mParticleDataTex = gl::Texture(infoSurface, format);
	mParticleDataTex.setWrap( GL_REPEAT, GL_REPEAT );
	mParticleDataTex.setMinFilter( GL_NEAREST );
	mParticleDataTex.setMagFilter( GL_NEAREST );
}

void Dust::setupVBO()
{
    // A dummy VboMesh the same size as the texture to keep the vertices on the GPU
    vector<Vec2f> texCoords;
    vector<uint32_t> indices;
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    layout.setStaticTexCoords2d();
    layout.setStaticNormals();
    
    mVboMesh = gl::VboMesh( kNumParticles, kNumParticles, layout, GL_POINTS);
    for( int x = 0; x < kBufSize; ++x ) {
        for( int y = 0; y < kBufSize; ++y ) {
            indices.push_back( x * kBufSize + y );
            texCoords.push_back( Vec2f( x/(float)kBufSize, y/(float)kBufSize ) );
        }
    }
    mVboMesh.bufferIndices( indices );
    mVboMesh.bufferTexCoords2d( 0, texCoords );
    mVboMesh.unbindBuffers();
}

//vector<Surface32f> Dust::generateInitialSurfaces()
//{
//}

void Dust::reset()
{
    mReset = true;
}

#pragma mark - Interface

void Dust::setupInterface()
{
    mInterface->addParam(CreateFloatParam( "timestep", &mTimeStep )
                         .minValue(0.001f)
                         .maxValue(3.0f)
                         .oscReceiver(mName));
    
    mInterface->addParam(CreateFloatParam( "point_size", &mPointSize )
                         .minValue(0.01f)
                         .maxValue(3.0f)
                         .oscReceiver(mName));
    
    mInterface->addParam(CreateFloatParam( "decay_rate", &mDecayRate )
                         .minValue(0.0f)
                         .maxValue(1.0f)
                         .oscReceiver(mName));
    
    mInterface->addParam(CreateBoolParam("dynamic_noise", &mUseDynamicTex)
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam("noise_speed", &mNoiseSpeed )
                         .maxValue(1.0f)
                         .oscReceiver(mName));
    mInterface->addParam(CreateVec3fParam("noise", &mNoiseScale, Vec3f::zero(), Vec3f(10.0f,10.0f,1.0f))
                         .oscReceiver(mName));
    
    mInterface->addParam(CreateBoolParam("audioreactive", &mAudioReactive)
                         .oscReceiver(mName));
    //mInterface->addParam(CreateBoolParam("audiospeed", &mAudioTime));
    
    mAudioInputHandler.setupInterface(mInterface, mName);
}

#pragma mark - Update

void Dust::update(double dt)
{
    mAudioInputHandler.update(dt, mApp->getAudioInput(), mGain);
    
    gl::disableAlphaBlending();
    
    gl::disableAlphaBlending();
	gl::disableDepthRead();
	gl::disableDepthWrite();
    mNoiseTheta += dt * mNoiseSpeed;
    //float time = (float)getElapsedSeconds();
	//mNoiseTheta = time;
    generateDynamicTexture();
    
    gl::pushMatrices();
    gl::setMatricesWindow( mParticlesFbo.getSize(), false ); // false to prevent vertical flipping
    gl::setViewport( mParticlesFbo.getBounds() );
    
    mParticlesFbo.bindUpdate();
    
    mParticleDataTex.bind(2);
    mInitialVelTex.bind(3);
    mInitialPosTex.bind(4);
    //mNoiseTex.bind(5);
    
    if (mUseDynamicTex)
    {
        mDynamicTexFbo.bindTexture(5);
    }
    else
    {
        mNoiseTex.bind(5);
    }
    
    float simdt = (float)(dt*mTimeStep);
    float decayRate = mDecayRate;
    if (mAudioTime) simdt *= (1.0 - mAudioInputHandler.getAverageVolumeLowFreq());
    if (mAudioTime) decayRate = mDecayRate + mAudioInputHandler.getAverageVolumeMidFreq();
    mSimulationShader.bind();
    mSimulationShader.uniform( "positions", 0 );
    mSimulationShader.uniform( "velocities", 1 );
    mSimulationShader.uniform( "information", 2);
	mSimulationShader.uniform( "oVelocities", 3);
	mSimulationShader.uniform( "oPositions", 4);
  	mSimulationShader.uniform( "noiseTex", 5);
    mSimulationShader.uniform( "dt", simdt );
    mSimulationShader.uniform( "decayRate", decayRate );
    mSimulationShader.uniform( "reset", mReset );
    //mSimulationShader.uniform( "takeFormation", mTakeFormation );
    
    gl::drawSolidRect(mParticlesFbo.getBounds());
    mSimulationShader.unbind();
    
    if (mUseDynamicTex)
    {
        mDynamicTexFbo.unbindTexture();
    }
    else
    {
        mNoiseTex.unbind();
    }
    mInitialPosTex.unbind();
    mInitialVelTex.unbind();
    mParticleDataTex.unbind();
    
    mParticlesFbo.unbindUpdate();
    gl::popMatrices();
    
    mReset = false;
    
    Scene::update(dt);
}

#pragma mark - Draw

void Dust::draw()
{
    gl::pushMatrices();
    glPushAttrib( GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT );
    
    //gl::setMatrices( getCamera() );
    gl::setMatricesWindow( getWindowSize() );
    gl::setViewport( mApp->getViewportBounds() );
    
    gl::enable(GL_POINT_SPRITE);
    gl::enable(GL_PROGRAM_POINT_SIZE_EXT);
    glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    
    gl::enableAlphaBlending();
    //gl::enableAdditiveBlending();
//    gl::disableDepthWrite();
//    gl::disableDepthRead();
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    glEnable(GL_TEXTURE_2D);
    
    mParticlesFbo.bindTexture(0);
    mParticlesFbo.bindTexture(1);
    mSpriteTex.bind(2);
    mParticleDataTex.bind(3);
    
    mRenderShader.bind();
    mRenderShader.uniform("posMap", 0);
    mRenderShader.uniform("velMap", 1);
    mRenderShader.uniform("spriteTex", 2);
    mRenderShader.uniform("information", 3);
    mRenderShader.uniform("screenWidth", (float)kBufSize);
    mRenderShader.uniform("spriteWidth", mPointSize);
    
    mRenderShader.uniform("audioReactive", mAudioReactive);
    if (mAudioReactive)
    {
        const int NUM_TRACKS = 4;
        Vec3f trackLevels[NUM_TRACKS];
//        for( int i = 0; i < NUM_TRACKS; ++i )
//        {
//            trackLevels[i] = Vec3f(mApp->getAudioInput().getLowLevelForLiveTrack(i),
//                                   mApp->getAudioInput().getMidLevelForLiveTrack(i),
//                                   mApp->getAudioInput().getHighLevelForLiveTrack(i));
//        }
        trackLevels[0] = Vec3f(mAudioInputHandler.getAverageVolumeLowFreq(),
                               mAudioInputHandler.getAverageVolumeMidFreq(),
                               mAudioInputHandler.getAverageVolumeHighFreq());
        mRenderShader.uniform("trackAudio1", trackLevels[0]);
//        mRenderShader.uniform("trackAudio2", trackLevels[1]);
//        mRenderShader.uniform("trackAudio3", trackLevels[2]);
//        mRenderShader.uniform("trackAudio4", trackLevels[3]);
    }
    
    glScalef(mApp->getViewportWidth() / (float)kBufSize , mApp->getViewportHeight() / (float)kBufSize ,1.0f);
    
    gl::draw( mVboMesh );
    
    mRenderShader.unbind();
    mParticleDataTex.unbind();
    mSpriteTex.unbind();
    mParticlesFbo.unbindTexture();
    
    glPopAttrib();
    gl::popMatrices();
}

void Dust::drawDebug()
{
    Scene::drawDebug();
    
    gl::pushMatrices();
    glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
    gl::enable( GL_TEXTURE_2D );
    const Vec2f& windowSize( getWindowSize() );
    gl::setMatricesWindow( windowSize );
    
    const float size = 80.0f;
    const float paddingX = 20.0f;
    const float paddingY = 240.0f;
    
    Rectf preview( windowSize.x - (size+paddingX), windowSize.y - (size+paddingY), windowSize.x-paddingX, windowSize.y - paddingY );
    gl::draw( mParticlesFbo.getTexture(0), preview );
    
    Rectf preview2 = preview - Vec2f(size+paddingX, 0.0f);
    gl::draw( mParticlesFbo.getTexture(1), preview2 );
    
    Rectf preview3 = preview2 - Vec2f(size+paddingX, 0.0f);
    if (mUseDynamicTex)
    {
        gl::draw(mDynamicTexFbo.getTexture(), preview3);
    }
    else
    {
        gl::draw(mNoiseTex, preview3);
    }
    
    glPopAttrib();
    gl::popMatrices();
}

#pragma mark - Texture Geneator

void Dust::generateDynamicTexture()
{
    gl::pushMatrices();
    
    // Bind FBO and set up window
	mDynamicTexFbo.bindFramebuffer();
	gl::setViewport( mDynamicTexFbo.getBounds() );
	gl::setMatricesWindow( mDynamicTexFbo.getSize() );
	gl::clear();
    
	// Bind and configure dynamic texture shader
	mDynamicTexShader.bind();
	mDynamicTexShader.uniform( "theta", mNoiseTheta );
    mDynamicTexShader.uniform( "scale", mNoiseScale );
    
	// Draw shader output
	gl::enable( GL_TEXTURE_2D );
	gl::color( Colorf::white() );
	gl::begin( GL_TRIANGLES );
    
     // TODO: cleanup
	// Define quad vertices
	Vec2f vert0( (float)mDynamicTexFbo.getBounds().x1, (float)mDynamicTexFbo.getBounds().y1 );
	Vec2f vert1( (float)mDynamicTexFbo.getBounds().x2, (float)mDynamicTexFbo.getBounds().y1 );
	Vec2f vert2( (float)mDynamicTexFbo.getBounds().x1, (float)mDynamicTexFbo.getBounds().y2 );
	Vec2f vert3( (float)mDynamicTexFbo.getBounds().x2, (float)mDynamicTexFbo.getBounds().y2 );
    
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
	mDynamicTexShader.unbind();
	mDynamicTexFbo.unbindFramebuffer();
    
    gl::popMatrices();
    
	///////////////////////////////////////////////////////////////
}