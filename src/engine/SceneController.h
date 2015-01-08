//
//  SceneController.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-12-12.
//
//

#pragma once

#include "Scene.h"
#include <vector>
#include <unordered_map>

namespace oculon
{
    
class SceneController
{
public:
    SceneController();
    ~SceneController();
    
    void addScene( Scene::Ref scene );
    
private:
    typedef std::vector<Scene::Ref> SceneList;
    
    SceneList               mScenes;
    Scene::NamedObjectMap   mScenesByName;
};
    
}
