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
, mCurrentFormationIndex(0)
, mCurrentRendererIndex(0)
{
}

ParticleController::~ParticleController()
{
    for(auto formation : mFormations)
    {
        delete formation;
    }
    mFormations.clear();
    
    for(auto renderer : mRenderers)
    {
        delete renderer;
    }
    mRenderers.clear();
}

#pragma mark - setup

void ParticleController::setup(int bufSize)
{
    mFboSize = bufSize;
    mNumParticles = mFboSize * mFboSize;
    
    // setup the framebuffers
    // bufSize x bufSize
    Surface32f posSurface = Surface32f(bufSize,bufSize,true);
	Surface32f velSurface = Surface32f(bufSize,bufSize,true);
	Surface32f dataSurface = Surface32f(bufSize,bufSize,true);
    
    Surface32f::Iter surfaceIter = posSurface.getIter();
    
    // encode the values as colors
    while(surfaceIter.line())
	{
		while(surfaceIter.pixel())
		{
            posSurface.setPixel(surfaceIter.getPos(), ColorA::white());
            velSurface.setPixel(surfaceIter.getPos(), ColorA::white());
            dataSurface.setPixel(surfaceIter.getPos(), ColorA::white());
        }
    }
    
    // create ping-pong fbo with multiple texture channels
    std::vector<Surface32f> surfaces;
    surfaces.push_back( posSurface );
    surfaces.push_back( velSurface );
    surfaces.push_back( dataSurface );
    mParticlesFbo = PingPongFbo( surfaces );
}

void ParticleController::setupInterface(Interface *interface, const std::string &name)
{
    vector<string> formationNames = getFormationNames();
    interface->addEnum(CreateEnumParam( "formation", (int*)(&mCurrentFormationIndex) )
                        .maxValue(formationNames.size())
                        .isVertical()
                        .oscReceiver(name)
                        .sendFeedback(), formationNames)->registerCallback(this, &ParticleController::onFormationChange);
    
    interface->gui()->addSeparator();
    vector<string> rendererNames = getRendererNames();
    interface->addEnum(CreateEnumParam( "renderer", (int*)(&mCurrentRendererIndex) )
                       .maxValue(rendererNames.size())
                       .isVertical()
                       .oscReceiver(name)
                       .sendFeedback(), rendererNames);
    // TODO: panels?
    for( vector<ParticleRenderer*>::const_reference renderer: mRenderers )
    {
        renderer->setupInterface(interface, name);
    }
}

#pragma mark - Formations

ParticleFormation& ParticleController::getFormation()
{
    assert(mCurrentFormationIndex < mFormations.size());
    return *(mFormations[mCurrentFormationIndex]);
}

void ParticleController::addFormation(ParticleFormation* formation)
{
    assert(formation);
    mFormations.push_back(formation);
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
            posIter++;
            
            ColorAf velPixel = ColorAf( (*velIter).x, (*velIter).y, (*velIter).z, (*velIter).w );
            velSurface.setPixel(surfaceIter.getPos(), velPixel);
            velIter++;
            
            ColorAf dataPixel = ColorAf( (*dataIter).x, (*dataIter).y, (*dataIter).z, (*dataIter).w );
            dataSurface.setPixel(surfaceIter.getPos(), dataPixel);
            dataIter++;
        }
    }
    
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
    
    mFormations.push_back( new ParticleFormation(name,posTex,velTex,dataTex) );
}

const std::vector<std::string> ParticleController::getFormationNames()
{
    vector<string> names;
    for( vector<ParticleFormation*>::const_reference formation: mFormations )
    {
        names.push_back(formation->getName());
    }
    return names;
}

void ParticleController::resetToFormation(const int formationIndex, const int resetFlags)
{
    assert(formationIndex < mFormations.size());
    
    if (resetFlags & RESET_POSITION)
    {
        mParticlesFbo.setTexture( 0, mFormations[formationIndex]->getPositionTex() );
    }
    if (resetFlags & RESET_VELOCITY)
    {
        mParticlesFbo.setTexture( 1, mFormations[formationIndex]->getVelocityTex() );
    }
    if (resetFlags & RESET_DATA)
    {
        mParticlesFbo.setTexture( 2, mFormations[formationIndex]->getDataTex() );
    }
    
    mParticlesFbo.reset();
}

bool ParticleController::onFormationChange()
{
    mFormationChangedSignal();
    return true;
}

#pragma mark - Renderers

ParticleRenderer& ParticleController::getRenderer()
{
    assert(mCurrentRendererIndex < mRenderers.size());
    return *(mRenderers[mCurrentRendererIndex]);
}

void ParticleController::addRenderer(ParticleRenderer* renderer)
{
    assert(renderer);
    renderer->setup(mFboSize);
    mRenderers.push_back(renderer);
}

const std::vector<std::string> ParticleController::getRendererNames()
{
    vector<string> names;
    for( vector<ParticleRenderer*>::const_reference renderer: mRenderers )
    {
        names.push_back(renderer->getName());
    }
    return names;
}

#pragma mark - update

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

#pragma mark - draw

void ParticleController::draw(const ci::Vec2i &screenSize, const ci::Camera &cam, AudioInputHandler &audioInputHandler)
{
    getRenderer().draw(mParticlesFbo, screenSize, cam, audioInputHandler);
}
