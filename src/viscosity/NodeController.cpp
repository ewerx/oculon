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
        //gl::drawSolidCircle(node.mPosition.xy(), 10.0f);
    }
}

#pragma mark - Mirror Bounce

MirrorBounceFormation::MirrorBounceFormation()
: NodeFormation("mirrorbounce")
, mRadius(1.0f)
, mBounceMultiplier(1.0f)
, mNumNodes(2)
, mRandomizeDirection(false)
, mRandomizeNext(false)
, mSpinRate(0.0f)
, mSpinTheta(0.0f)
, mSpinCenter(Vec3f::zero())
, mMirrorAxis(Vec3f::xAxis())
{
    mNodes.push_back( NodeFormation::Node( Vec3f::zero(), mMirrorAxis ) );
    mNodes.push_back( NodeFormation::Node( Vec3f::zero(), -mMirrorAxis ) );
    
    mNodes.push_back( NodeFormation::Node( Vec3f::zero(), Vec3f( -mMirrorAxis.x, mMirrorAxis.y, mMirrorAxis.z) ) );
    mNodes.push_back( NodeFormation::Node( Vec3f::zero(), -Vec3f( -mMirrorAxis.x, mMirrorAxis.y, mMirrorAxis.z) ) );
    
    mAudioReactive = true;
}

void MirrorBounceFormation::setupInterface(Interface *interface, const std::string &prefix)
{
    NodeFormation::setupInterface(interface, prefix);
    
    string name = prefix + "/" + mName;
    
    interface->addParam(CreateBoolParam("audio_reactive", &mAudioReactive)
                        .oscReceiver(name));
    
//    interface->addParam(CreateIntParam("num_nodes", &mNumNodes)
//                        .minValue(2)
//                        .maxValue(6))->registerCallback(this, &MirrorBounceFormation::onNumNodesChanged);
    
    interface->addParam(CreateBoolParam("randomize_dir", &mRandomizeDirection)
                        .oscReceiver(name));
    
    interface->addParam(CreateVec3fParam("mirror_axis", &mMirrorAxis, Vec3f::zero(), Vec3f::one())
                        .oscReceiver(name))->registerCallback(this, &MirrorBounceFormation::onMirrorAxisChanged);
    
    interface->addParam(CreateFloatParam("spinrate", &mSpinRate)
                         .minValue(-2.0f)
                         .maxValue(2.0f));
    
    interface->addParam(CreateFloatParam("radius", &mRadius)
                        .minValue(1.0f)
                        .maxValue(1000.0f)
                        .oscReceiver(name)
                        .midiInput(0, 1, 24));
    
    interface->addParam(CreateFloatParam("bounce_multi", &mBounceMultiplier)
                        .minValue(1.0f)
                        .maxValue(100.0f)
                        .oscReceiver(name)
                        .midiInput(0, 2, 24));
    
    // TODO: control radius externally?
}

bool MirrorBounceFormation::onMirrorAxisChanged()
{
    mNodes[0].mVelocity = mMirrorAxis;
    mNodes[1].mVelocity = -mMirrorAxis;
    mNodes[2].mVelocity = Vec3f( -mMirrorAxis.x, mMirrorAxis.y, mMirrorAxis.z);
    mNodes[3].mVelocity = -Vec3f( -mMirrorAxis.x, mMirrorAxis.y, mMirrorAxis.z);
    return true;
}

//bool MirrorBounceFormation::onNumNodesChanged()
//{
//    if (mNumNodes > mNodes.size())
//    {
//        int numToAdd = mNumNodes - mNodes.size();
//        float mirror = (mNodes.size() % 2 == 0) ? 1.0f : -1.0f;
//        Vec3f axis = mMirrorAxis.cross(-mMirrorAxis);
//        for (int i = 0; i < numToAdd; ++i)
//        {
//            mNodes.push_back( NodeFormation::Node( Vec3f::zero(), axis * mirror ) );
//            mirror *= -1.0f;
//        }
//    }
//    else
//    {
//        int numToRemove = mNodes.size() - mNumNodes;
//        mNodes.pop_back()
//    }
//}

void MirrorBounceFormation::update(double dt, AudioInputHandler &audioInputHandler)
{
    mSpinTheta += dt * mSpinRate;
    mSpinRho += dt * mSpinRate * 0.3333f;
    
    if (mAudioReactive)
    {
        float distance = audioInputHandler.getAverageVolumeLowFreq();
        float radius = distance * mRadius * mBounceMultiplier;
        
        if (mRandomizeDirection)
        {
            if (mRandomizeNext && radius < (0.25f * mRadius * mBounceMultiplier))
            {
                mNodes[0].mVelocity = Rand::randVec3f().normalized();
                mNodes[1].mVelocity = mNodes[0].mVelocity * -1.0f;
                mNodes[2].mVelocity = Vec3f( -mNodes[0].mVelocity.x, -mNodes[0].mVelocity.y, mNodes[0].mVelocity.z * (Rand::randBool() ? 1.0f : -1.0f));
                mNodes[3].mVelocity = Vec3f( -mNodes[1].mVelocity.x, -mNodes[1].mVelocity.y, mNodes[1].mVelocity.z * (Rand::randBool() ? 1.0f : -1.0f));
                mRandomizeNext = false;
            }
            else if (radius > (0.5f * mRadius * mBounceMultiplier))
            {
                mRandomizeNext = true;
            }
        }
        
        float mirror = 1.0f;
        float mirror2 = 1.0f;
        for ( tNodeList::reference node : mNodes )
        {
            if ( mSpinRate > 0.0f )
            {
                node.mPosition.x = mSpinCenter.x + cos( mSpinRho * mirror2 ) * sin( mSpinTheta ) * radius * mirror;
                node.mPosition.y = mSpinCenter.y + sin( mSpinRho * mirror2 ) * sin( mSpinTheta ) * radius * mirror;
                node.mPosition.z = mSpinCenter.z + cos( mSpinTheta * mirror2 ) * radius * mirror;
                node.mVelocity = node.mPosition;
                
                mirror *= -1.0f;
                if (mirror > 0.0f)
                {
                    mirror2 *= -1.0f;
                }
            }
            else
            {
                node.mPosition = node.mVelocity * radius;
            }
        }
    }
    else
    {
        // TODO: timeline-based?
    }
}
