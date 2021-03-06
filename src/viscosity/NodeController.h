//
//  NodeController.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-11.
//
//

#pragma once

#include "AudioInputHandler.h"

#pragma mark - Formations

class NodeFormation
{
public:
    class Node
    {
    public:
        Node();
        Node(const ci::Vec3f& pos, const ci::Vec3f& vel);
        
    public:
        ci::Vec3f mPosition;
        ci::Vec3f mVelocity;
    };
    
public:
    NodeFormation(const std::string& name);
    virtual ~NodeFormation() {};
    
    virtual void setupInterface( Interface* interface, const std::string& prefix );
    virtual void update(double dt, AudioInputHandler& audioInputHandler) {};
    virtual void draw();
    virtual void drawDebug();
    
    const std::string& getName() const { return mName; }
    
    typedef std::vector<Node> tNodeList;
    tNodeList& getNodes() { return mNodes; }
    
protected:
    std::string mName;
    
    tNodeList mNodes;
    
    bool mAudioReactive;
};

// two nodes repelling each other on an axis
class MirrorBounceFormation : public NodeFormation
{
public:
    MirrorBounceFormation();
    ~MirrorBounceFormation() {}
    
    void setupInterface( Interface* interface, const std::string& prefix );
    void update(double dt, AudioInputHandler& audioInputHandler);
    
    // callbacks
    bool onMirrorAxisChanged();
//    bool onNumNodesChanged();
    
public:
    float mRadius;
    float mBounceMultiplier;
    bool mRandomizeDirection;
    float mSpinRate;
    ci::Vec3f mSpinCenter;
    ci::Vec3f mMirrorAxis;
    
private:
    int mNumNodes;
    bool mRandomizeNext;
    float mSpinTheta;
    float mSpinRho;
};

#pragma mark - Node Controller

class NodeController
{
public:
    NodeController();
    virtual ~NodeController();
    
    void addFormation( NodeFormation* formation );
    const std::vector<std::string> getFormationNames();
    
    void setupInterface( Interface* interface, const std::string& name );
    void update(double dt, AudioInputHandler& audioInputHandler);
    void draw();
    void drawDebug();
    
    NodeFormation::tNodeList& getNodes();
    
    void setFormation( const int index )
    {
        if ( index < mFormations.size() )
            mCurrentFormationIndex = index;
    }
    
    // callbacks
    bool onFormationChanged();

private:
    typedef std::vector<NodeFormation*> tNodeFormationList;
    tNodeFormationList mFormations;
    int mCurrentFormationIndex;
};

