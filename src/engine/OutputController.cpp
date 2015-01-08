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
    mChannelsByName.emplace(make_pair("window", OutputChannel::Ref(new WindowOutputChannel("window", getWindowWidth(), getWindowHeight()))));

    // syphon servers

    // movie writer

    // frame saver

    // oculus rift
}

#pragma mark - Assignment

bool OutputController::assignSceneToOutput(const Scene::Ref scene, const std::string channelName)
{
    bool result = false;
    
    if (mChannelsByName.find(channelName) != mChannelsByName.end())
    {
        mScenesByChannelName[channelName] = scene;
    }
    else
    {
        assert(false && "assignSceneToChannel: invalid channel");
    }
    
    return result;
}

bool OutputController::unassignSceneFromAllOutputs(const Scene::Ref scene)
{
    bool result = false;
    
    for (auto &channelNameScenePair : mScenesByChannelName)
    {
        const string& channelName = channelNameScenePair.first;
        if (mScenesByChannelName[channelName] == channelNameScenePair.second)
        {
            mScenesByChannelName[channelName] = nullptr;
            result = true;
        }
    }
    
    return result;
}

bool OutputController::unassignSceneFromOutput(const std::string channelName)
{
    bool result = false;
    
    if (mScenesByChannelName.find(channelName) != mScenesByChannelName.end())
    {
        mScenesByChannelName[channelName] = nullptr;
    }
    else
    {
        assert(false && "assignSceneToChannel: invalid channel");
    }
    
    return result;
}

bool OutputController::assignSceneToSyphon(Scene::Ref scene)
{
    // TODO
    assert(false && "not implemented");
    
    return false;
}

#pragma mark - Output

void OutputController::outputScenesToAllChannels()
{
    // order is undefined
    for (auto &channelNameScenePair : mScenesByChannelName)
    {
        const string& channelName = channelNameScenePair.first;
        const Scene::Ref& scene = channelNameScenePair.second;
        
        assert(mChannelsByName.find(channelName) != mChannelsByName.end());
        
        mChannelsByName[channelName]->outputFrame(make_shared<Texture>(scene->getFboTexture()));
    }
}

