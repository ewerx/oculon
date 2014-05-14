//
//  NodeController.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-11.
//
//

#include "NodeController.h"
#include "Interface.h"

#include "cinder/Rand.h"

using namespace ci;
using namespace std;

#pragma mark - Node Controller

NodeController::NodeController()
: mCurrentFormationIndex(0)
{
    mFormations.push_back( new NodeFormation("none") );
}

NodeController::~NodeController()
{
    for(auto formation : mFormations)
    {
        delete formation;
    }
    mFormations.clear();
}

void NodeController::addFormation(NodeFormation *formation)
{
    assert(formation);
    mFormations.push_back(formation);
}

const std::vector<std::string> NodeController::getFormationNames()
{
    vector<string> names;
    for( vector<NodeFormation*>::const_reference formation: mFormations )
    {
        names.push_back(formation->getName());
    }
    return names;
}

void NodeController::setupInterface(Interface *interface, const std::string &name)
{
    vector<string> formationNames = getFormationNames();
    interface->addEnum(CreateEnumParam( "node_formation", (int*)(&mCurrentFormationIndex) )
                       .maxValue(formationNames.size())
                       .isVertical()
                       .oscReceiver(name)
                       .sendFeedback(), formationNames)->registerCallback(this, &NodeController::onFormationChanged);
    
    // TODO: panels?
    for( tNodeFormationList::const_reference formation: mFormations )
    {
        formation->setupInterface(interface, name);
    }
}

bool NodeController::onFormationChanged()
{
    return true;
}

void NodeController::update(double dt, AudioInputHandler &audioInputHandler)
{
    mFormations[mCurrentFormationIndex]->update(dt, audioInputHandler);
}

void NodeController::draw()
{
    mFormations[mCurrentFormationIndex]->draw();
}

void NodeController::drawDebug()
{
    mFormations[mCurrentFormationIndex]->drawDebug();
}

NodeFormation::tNodeList& NodeController::getNodes()
{
    assert( mCurrentFormationIndex < mFormations.size() );
    return mFormations[mCurrentFormationIndex]->getNodes();
}

#pragma mark - Base Node

NodeFormation::Node::Node()
: Node(Vec3f::zero(), Vec3f::zero())
{
}

NodeFormation::Node::Node(const ci::Vec3f& pos, const ci::Vec3f& vel)
: mPosition(pos)
, mVelocity(vel)
{
}

#pragma mark - Base Formation

NodeFormation::NodeFormation(const std::string& name)
: mName(name)
, mAudioReactive(false)
{
}

void NodeFormation::setupInterface(Interface *interface, const std::string &prefix)
{
    string name = prefix + "/" + mName;
    interface->addParam(CreateBoolParam("audio_reactive", &mAudioReactive)
                        .oscReceiver(name));
    
}

void NodeFormation::draw()
{
    // TODO: what do nodes look like?
}

void NodeFormation::drawDebug()
{
    for ( tNodeList::reference node : mNodes )
    {
        gl::color(1.0f, 0.0f, 0.0f);
        gl::drawSphere(node.mPosition, 2.0f);
        gl::drawSolidCircle(node.mPosition.xy(), 10.0f);
    }
}

#pragma mark - Mirror Bounce

MirrorBounceFormation::MirrorBounceFormation(float radius)
: NodeFormation("mirrorbounce")
, mRadius(radius)
, mRandomizeDirection(false)
, mRandomizeNext(false)
, mSpinRate(0.0f)
, mSpinTheta(0.0f)
, mSpinCenter(Vec3f::zero())
, mMirrorAxis(Vec3f::xAxis())
{
    mNodes.push_back( NodeFormation::Node( Vec3f::zero(), mMirrorAxis ) );
    mNodes.push_back( NodeFormation::Node( Vec3f::zero(), -mMirrorAxis ) );
    
    mAudioReactive = true;
}

void MirrorBounceFormation::setupInterface(Interface *interface, const std::string &prefix)
{
    NodeFormation::setupInterface(interface, prefix);
    
    string name = prefix + "/" + mName;
    interface->addParam(CreateBoolParam("randomize_dir", &mRandomizeDirection)
                        .oscReceiver(name));
    
    interface->addParam(CreateVec3fParam("mirror_axis", &mMirrorAxis, Vec3f::zero(), Vec3f::one())
                        .oscReceiver(name))->registerCallback(this, &MirrorBounceFormation::onMirrorAxisChanged);
    
    interface->addParam(CreateFloatParam("spinrate", &mSpinRate)
                         .minValue(-2.0f)
                         .maxValue(2.0f));
    
    // TODO: control radius externally?
}

bool MirrorBounceFormation::onMirrorAxisChanged()
{
    mNodes[0].mVelocity = mMirrorAxis;
    mNodes[1].mVelocity = -mMirrorAxis;
    return true;
}

void MirrorBounceFormation::update(double dt, AudioInputHandler &audioInputHandler)
{
    mSpinTheta += dt * mSpinRate;
    
    if (mAudioReactive)
    {
        float distance = audioInputHandler.getAverageVolumeLowFreq();
        
        if (mRandomizeDirection)
        {
            if (mRandomizeNext && distance < 0.1f)
            {
                mNodes[0].mVelocity = Rand::randVec3f().normalized();
                mNodes[1].mVelocity = mNodes[0].mVelocity * -1.0f;
                mRandomizeNext = false;
            }
            else if (distance > 0.5f)
            {
                mRandomizeNext = true;
            }
        }
        
        float mirror = 1.0f;
        for ( tNodeList::reference node : mNodes )
        {
            if ( mSpinRate > 0.0f )
            {
                node.mPosition.x = mSpinCenter.x + distance * sin( mSpinTheta ) * mirror;
                node.mPosition.y = mSpinCenter.y + distance * cos( mSpinTheta ) * mirror;
                node.mVelocity = node.mPosition;
                mirror *= -1.0f;
            }
            else
            {
                node.mPosition = node.mVelocity * distance;
            }
        }
    }
    else
    {
        // TODO: timeline-based?
    }
}
