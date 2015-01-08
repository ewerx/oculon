//
//  OutputController.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2015-01-06.
//
//

#include "OutputController.h"

using namespace oculon;
using namespace ci;
using namespace ci::app;
using namespace ci::gl;
using namespace std;

OutputController::OutputController()
{
    mChannels.emplace(make_pair("window", OutputChannel::Ref(new WindowOutputChannel("window", getWindowWidth(), getWindowHeight()))));

    // syphon servers

    // movie writer

    // frame saver

    // oculus rift
}

bool OutputController::assignChannelToScene(const std::string channelName, const Scene::Ref scene)
{
    bool result = false;

    if (mChannels.find(channelName) != mChannels.end())
    {
        if (mSceneOutputChannels.find(scene) != mSceneOutputChannels.end())
        {
            const OutputChannelList &channels = mSceneOutputChannels[scene];
            if (!vectorContains(channels, mChannels[channelName]))
            {
                mSceneOutputChannels[scene].push_back(mChannels[channelName]);
            }
        }
        else
        {
            mSceneOutputChannels.emplace(make_pair(scene, mChannels[channelName]));
        }
        result = true;
    }
    else
    {
        assert(false && "assignSceneToChannel: invalid channel");
    }

    return result;
}

bool OutputController::unassignChannelFromScene(const std::string channelName, const Scene::Ref scene)
{
    bool result = false;
    
    if (mChannels.find(channelName) != mChannels.end())
    {
        //SceneOutputMap::iterator sceneMapIter = mSceneOutputChannels.find(scene);
        
        if (mSceneOutputChannels.find(scene) != mSceneOutputChannels.end() &&
            vectorContains(mSceneOutputChannels[scene], mChannels[channelName]))
        {
            //OutputChannelList &channels = mSceneOutputChannels[scene];
            //OutputChannelList::iterator channelIter = find(channels.begin(), channels.end(), mChannels[channelName]);
            vectorRemove(mSceneOutputChannels[scene], mChannels[channelName]);
            //mSceneOutputChannels[scene].erase(remove(channels.begin(), channels.end(), mChannels[channelName]));
            result = true;
        }
        else
        {
            assert(false && "assignSceneToChannel: channel not assigned to scene");
        }
    }
    else
    {
        assert(false && "assignSceneToChannel: invalid channel");
    }
    
    return result;
}

bool OutputController::assignSceneToSyphonChannel(Scene::Ref scene)
{
    // TODO
    assert(false && "not implemented");
    
    return false;
}

// MARK: output

void OutputController::outputScenesToAllChannels()
{
    // order is undefined
    for (const auto& sceneChannelsPair : mSceneOutputChannels)
    {
        const Scene::Ref& scene = sceneChannelsPair.first;
        const OutputChannelList& channels = sceneChannelsPair.second;
        
        for (const auto& channel : channels)
        {
            channel->outputFrame(make_shared<Texture>(scene->getFboTexture()));
        }
    }
}

