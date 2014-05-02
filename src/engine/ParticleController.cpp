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
: mFboSize(0)
, mNumParticles(0)
{
}

ParticleController::~ParticleController()
{
}

#pragma mark - setup

void ParticleController::setup(int bufSize)
{
    mFboSize = bufSize;
    mNumParticles = mFboSize * mFboSize;
    
//    assert(initPositions.size() == initVelocities.size() == initData.size() == mNumParticles);
    
    // setup the framebuffers
    // bufSize x bufSize
    Surface32f posSurface = Surface32f(bufSize,bufSize,true);
	Surface32f velSurface = Surface32f(bufSize,bufSize,true);
	Surface32f dataSurface = Surface32f(bufSize,bufSize,true);
    
    Surface32f::Iter surfaceIter = posSurface.getIter();
//    vector<Vec4f>::iterator posIter = initPositions.begin();
//    vector<Vec4f>::iterator velIter = initVelocities.begin();
//    vector<Vec4f>::iterator dataIter = initData.begin();
    
    // encode the values as colors
    while(surfaceIter.line())
	{
		while(surfaceIter.pixel())
		{
//            ColorAf posPixel = ColorAf( (*posIter).x, (*posIter).y, (*posIter).z, (*posIter).w );
//            posSurface.setPixel(surfaceIter.getPos(), posPixel);
//            
//            ColorAf velPixel = ColorAf( (*velIter).x, (*velIter).y, (*velIter).z, (*velIter).w );
//            velSurface.setPixel(surfaceIter.getPos(), velPixel);
//            
//            ColorAf dataPixel = ColorAf( (*dataIter).x, (*dataIter).y, (*dataIter).z, (*dataIter).w );
            dataSurface.setPixel(surfaceIter.getPos(), ColorA::zero());
        }
    }
    
    // create ping-pong fbo with multiple texture channels
    std::vector<Surface32f> surfaces;
    surfaces.push_back( posSurface );
    surfaces.push_back( velSurface );
    surfaces.push_back( dataSurface );
    mParticlesFbo = PingPongFbo( surfaces );
}

void ParticleController::addFormation(const std::string &name,
                                      std::vector<ci::Vec4f> &positions,
                                      std::vector<ci::Vec4f> &velocities,
                                      std::vector<ci::Vec4f> &data)
{
    // setup the framebuffers
    // bufSize x bufSize
    Surface32f posSurface = Surface32f(mFboSize,mFboSize,true);
	Surface32f velSurface = Surface32f(mFboSize,mFboSize,true);
	Surface32f dataSurface = Surface32f(mFboSize,mFboSize,true);
    
    Surface32f::Iter surfaceIter = posSurface.getIter();
    vector<Vec4f>::iterator posIter = positions.begin();
    vector<Vec4f>::iterator velIter = velocities.begin();
    vector<Vec4f>::iterator dataIter = data.begin();
    
    // encode the values as colors
    while(surfaceIter.line())
	{
		while(surfaceIter.pixel())
		{
            ColorAf posPixel = ColorAf( (*posIter).x, (*posIter).y, (*posIter).z, (*posIter).w );
            posSurface.setPixel(surfaceIter.getPos(), posPixel);
            
            ColorAf velPixel = ColorAf( (*velIter).x, (*velIter).y, (*velIter).z, (*velIter).w );
            velSurface.setPixel(surfaceIter.getPos(), velPixel);
            
            ColorAf dataPixel = ColorAf( (*dataIter).x, (*dataIter).y, (*dataIter).z, (*dataIter).w );
            dataSurface.setPixel(surfaceIter.getPos(), dataPixel);
        }
    }
    
    // create ping-pong fbo with multiple texture channels
    std::vector<Surface32f> surfaces;
    surfaces.push_back( posSurface );
    surfaces.push_back( velSurface );
    surfaces.push_back( dataSurface );
    mParticlesFbo = PingPongFbo( surfaces );
    
    // store the original textures
    gl::Texture::Format format;
    format.setInternalFormat( GL_RGBA32F_ARB );
    
    gl::Texture posTex = gl::Texture(posSurface, format);
    posTex.setWrap( GL_REPEAT, GL_REPEAT );
    posTex.setMinFilter( GL_NEAREST );
    posTex.setMagFilter( GL_NEAREST );
    
    gl::Texture velTex = gl::Texture(velSurface, format);
    velTex.setWrap( GL_REPEAT, GL_REPEAT );
    velTex.setMinFilter( GL_NEAREST );
    velTex.setMagFilter( GL_NEAREST );
    
    gl::Texture dataTex = gl::Texture(dataSurface, format);
    dataTex.setWrap( GL_REPEAT, GL_REPEAT );
    dataTex.setMinFilter( GL_NEAREST );
    dataTex.setMagFilter( GL_NEAREST );
    
    mFormations.push_back( tFormation(name,posTex,velTex,dataTex) );
}

void ParticleController::resetToFormation(const int formationIndex, const int resetFlags)
{
    assert(formationIndex < mFormations.size());
    
    if (resetFlags & RESET_POSITION)
    {
        mParticlesFbo.setTexture(0, mFormations[formationIndex].mPositionTex);
    }
    if (resetFlags & RESET_VELOCITY)
    {
        mParticlesFbo.setTexture(1, mFormations[formationIndex].mVelocityTex);
    }
    if (resetFlags & RESET_DATA)
    {
        mParticlesFbo.setTexture(2, mFormations[formationIndex].mDataTex);
    }
    
    mParticlesFbo.reset();
}

void ParticleController::update(double dt)
{
    updateSimulation(dt);
}

void ParticleController::updateSimulation(double dt)
{
//    if (mCurrentBehavior < mBehaviors.size())
//    {
//        mBehaviors[mCurrentBehavior]->update(dt, mParticlesFbo);
//    }
}

void ParticleController::draw(const ci::Camera& cam)
{
//    if (mCurrentBehavior < mBehaviors.size())
//    {
//        // FIXME: need to update method signature to match ParticleRenderer::draw
//        //mRenderers[mCurrentBehavior]->draw(mParticlesFbo, cam);
//    }
    
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
