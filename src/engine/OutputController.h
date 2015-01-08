//
//  OutputController.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2015-01-06.
//
//

#pragma once

#include "OutputChannel.h"
#include "Scene.h"

namespace oculon
{
class OutputController
{
public:
    OutputController();
    
    bool assignSceneToOutput( const Scene::Ref scene, const std::string channelName );
    bool unassignSceneFromOutput( const std::string channelName );
    bool unassignSceneFromAllOutputs( const Scene::Ref scene );
    
    bool assignSceneToSyphon( Scene::Ref scene );
    
    void outputScenesToAllChannels();
    
private:
    typedef std::vector<OutputChannel::Ref> OutputChannelList;
    typedef std::unordered_map<Scene::Ref, OutputChannelList> SceneOutputMap;
    
    OutputChannel::NamedObjectMap mChannelsByName;
    Scene::NamedObjectMap mScenesByChannelName;
    
    SceneOutputMap mSceneOutputChannels;
};
    
}
