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
    
    void outputFrame( ci::gl::TextureRef tex );
    
    
    
private:
    typedef std::vector<OutputChannel::Ref> OutputChannelList;
    typedef std::vector<Scene::Ref> SceneList;
    typedef std::unordered_map<Scene::Ref, OutputChannelList> SceneOutputMap;
    
    OutputChannel::NamedObjectMap mChannels;
    
};
    
}
