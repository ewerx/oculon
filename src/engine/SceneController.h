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
    typedef std::vector<SceneRef> SceneList;
    typedef std::unordered_map<std::string, SceneRef> SceneMap;
    
    SceneList   mSceneList;
    SceneMap    mSceneMap;
};
    
}
