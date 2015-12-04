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
, mFormationStep(1.0f)
, mStartAnim(false)
{
    mFormationAnimSelector.mDuration = 10.0f;
    //mFormationAnimSelector.mSelectedCurve = EaseCurveSelector::EASE_OUTEXPO;
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
                        .sendFeedback(), formationNames)->registerCallback(this, &ParticleController::onFormationChanged);
    interface->addParam(CreateFloatParam( "formation_step", mFormationStep.ptr() )); // read-only
    mFormationAnimSelector.setupInterface(interface, name);
    
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
        interface->gui()->addColumn();
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

#pragma mark Callbacks

bool ParticleController::onFormationChanged()
{
    mFormationStep = 0.0f;
    mStartAnim = true;
    timeline().apply( &mFormationStep, 1.0f, mFormationAnimSelector.mDuration,mFormationAnimSelector.getEaseFunction() );
    
    mFormationChangedSignal();
    return true;
}

bool ParticleController::setFormation(const int formationIndex)
{
    mCurrentFormationIndex = formationIndex;
    onFormationChanged();
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
    assert(mFboSize > 0 && "ParticleController not setup");
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

bool ParticleController::isStartingAnim()
{
    bool starting = mStartAnim;
    mStartAnim = false;
    return starting;
}

#pragma mark - draw

void ParticleController::draw(const ci::Vec2i &viewportSize, const ci::Camera &cam, AudioInputHandler &audioInputHandler)
{
    getRenderer().draw(mParticlesFbo, viewportSize, cam, audioInputHandler);
}

void ParticleController::drawDebug(const ci::Vec2i &windowSize)
{
    gl::pushMatrices();
    glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
    gl::enable( GL_TEXTURE_2D );
    gl::setMatricesWindow( windowSize );
    
    const float size = 80.0f;
    const float paddingX = 20.0f;
    const float paddingY = 240.0f;
    
    Rectf preview( windowSize.x - (size+paddingX), windowSize.y - (size+paddingY), windowSize.x-paddingX, windowSize.y - paddingY );
    gl::draw( mParticlesFbo.getTexture(0), preview );
    
    Rectf preview2 = preview - Vec2f(size+paddingX, 0.0f);
    gl::draw( mParticlesFbo.getTexture(1), preview2 );
    
    Rectf preview3 = preview2 - Vec2f(size+paddingX, 0.0f);
    gl::draw( mParticlesFbo.getTexture(2), preview3 );
    
    glPopAttrib();
}
