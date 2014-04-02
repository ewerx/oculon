//
//  ParticleController.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2/11/2014.
//
//

#include "ParticleController.h"
#include "Utils.h"
#include "cinder/Rand.h"

using namespace std;
using namespace ci;
using namespace ci::app;

#pragma mark - construction

ParticleController::ParticleController()
: mFboSize(256)
, mNumParticles(mFboSize*mFboSize)
{
}

ParticleController::~ParticleController()
{
    for (ParticleRenderer *pr : mRenderers)
    {
        delete pr;
    }
    mRenderers.clear();
}

#pragma mark - setup

void ParticleController::setup(int fboSize)
{
    mFboSize = fboSize;
    mNumParticles = mFboSize * mFboSize;
    
    // add default behavior
    mBehaviors.push_back(new ParticleBehavior());
    
    setupFBO();
    //setupVBO();
    
    // add default renderer
    mRenderers.push_back(new ParticleRenderer());
    mRenderers[0]->setup(256);
}

void ParticleController::setupFBO()
{
    // shaders
    
//    mRenderShader = Utils::loadVertAndFragShaders("lines_render_vert.glsl", "lines_render_frag.glsl");
    
    // setup fbo
//    Surface32f posSurface = Surface32f(mFboSize,mFboSize,true);
//	Surface32f velSurface = Surface32f(mFboSize,mFboSize,true);
//	Surface32f infoSurface = Surface32f(mFboSize,mFboSize,true);
//    
//	Surface32f::Iter iterator = posSurface.getIter();
//    
//    // random positions with fixed seed
//    Rand rand(1010);
//    
//    while(iterator.line())
//	{
//		while(iterator.pixel())
//		{
//            float x = rand.randFloat(-1.0f,1.0f);
//            float y = rand.randFloat(-1.0f,1.0f);
//            float z = rand.randFloat(-1.0f,1.0f);
//            posSurface.setPixel(iterator.getPos(), ColorA(x,y,z,0.0f));
//			velSurface.setPixel(iterator.getPos(), ColorA(0.0f,0.0f,0.0f,0.0f));
//            infoSurface.setPixel(iterator.getPos(), ColorA(0.0f,0.0f,0.0f,0.0f));
//		}
//	}
//    
//    std::vector<Surface32f> surfaces;
//    surfaces.push_back( posSurface );
//    surfaces.push_back( velSurface );
//    surfaces.push_back( infoSurface );
    mParticlesFbo = PingPongFbo( 3, mFboSize );
}

//void ParticleController::setupVBO()
//{
//    // setup VBO
//    // A dummy VboMesh the same size as the texture to keep the vertices on the GPU
//    vector<Vec2f> texCoords;
//    vector<uint32_t> indices;
//    gl::VboMesh::Layout layout;
//    layout.setStaticIndices();
//    layout.setStaticPositions();
//    layout.setStaticTexCoords2d();
//    layout.setStaticNormals();
//    
//    mVboMesh = gl::VboMesh( mNumParticles, mNumParticles, layout, GL_LINES);
//    for( int y = 0; y < mFboSize; ++y ) {
//        for( int x = 0; x < mFboSize; ++x ) {
//            indices.push_back( y * mFboSize + x );
//            texCoords.push_back( Vec2f( x/(float)mFboSize, y/(float)mFboSize ) );
//        }
//    }
//    mVboMesh.bufferIndices( indices );
//    mVboMesh.bufferTexCoords2d( 0, texCoords );
//    mVboMesh.unbindBuffers();
//}

void ParticleController::update(double dt)
{
    updateSimulation(dt);
}

void ParticleController::updateSimulation(double dt)
{
    if (mCurrentBehavior < mBehaviors.size())
    {
        mBehaviors[mCurrentBehavior]->update(dt, mParticlesFbo);
    }
}

void ParticleController::draw(const ci::Camera& cam)
{
    if (mCurrentBehavior < mBehaviors.size())
    {
        // FIXME: need to update method signature to match ParticleRenderer::draw
        //mRenderers[mCurrentBehavior]->draw(mParticlesFbo, cam);
    }
    
//    glPushAttrib( GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT );
//    
//    //    gl::enable(GL_POINT_SPRITE);
//    //    gl::enable(GL_PROGRAM_POINT_SIZE_EXT);
//    //    glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
//    //    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
//    
//    //gl::enableAlphaBlending();
//    gl::enableAdditiveBlending();
//    gl::enableDepthRead();
//    gl::enableDepthWrite();
//    
//    glEnable(GL_TEXTURE_2D);
//    
//    gl::lineWidth(1.0f);
//    
//    mParticlesFbo.bindTexture(0);//pos
//    mParticlesFbo.bindTexture(1);//vel
//    mParticlesFbo.bindTexture(2);//info
//    
////    mColorMapTex.bind(3);
//    
////    if (mAudioReactive && mAudioInputHandler.hasTexture())
////    {
////        mAudioInputHandler.getFbo().bindTexture(4);
////    }
//    
//    mRenderShader.bind();
//    mRenderShader.uniform("posMap", 0);
//    mRenderShader.uniform("velMap", 1);
//    mRenderShader.uniform("information", 2);
//    mRenderShader.uniform("colorMap", 3);
//    mRenderShader.uniform("intensityMap", 4);
//    mRenderShader.uniform("gain", 1.0f);
//    mRenderShader.uniform("screenWidth", (float)mFboSize);
//    mRenderShader.uniform("colorBase", ColorA::white());
//    mRenderShader.uniform("audioReactive", false);
//    
//    const float scale = 10.0f;
//    //glScalef(scale, scale, scale);
//    
//    gl::draw( mVboMesh );
//    
//    mRenderShader.unbind();
////    mColorMapTex.unbind();
//    mParticlesFbo.unbindTexture();
//    
//    glPopAttrib();
}
