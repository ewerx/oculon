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
    
    bool assignChannelToScene( const std::string channelName, const Scene::Ref scene );
    bool unassignChannelFromScene( const std::string channelName, const Scene::Ref scene );
    
    bool assignSceneToSyphonChannel( Scene::Ref scene );
    
    bool removeChannel( const std::string channelName );
    
    void outputScenesToAllChannels();
    
    
    
private:
    typedef std::vector<OutputChannel::Ref> OutputChannelList;
    typedef std::unordered_map<Scene::Ref, OutputChannelList> SceneOutputMap;
    
    OutputChannel::NamedObjectMap mChannels;
    SceneOutputMap mSceneOutputChannels;
};
    
}
