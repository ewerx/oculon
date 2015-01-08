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
    
    
private:
    typedef std::vector<Scene::Ref> SceneList;
    
    SceneList               mSceneList;
    Scene::NamedObjectMap   mSceneMap;
};
    
}
