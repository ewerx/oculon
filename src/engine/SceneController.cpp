//
//  SceneController.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-12-12.
//
//

#include "SceneController.h"

using namespace oculon;

SceneController::SceneController()
{
    
}

SceneController::~SceneController()
{
    
}

void SceneController::addScene(Scene::Ref scene)
{
    assert(scene && "OculonApp::addScene: scene is null");
    //console() << (mScenes.size()+1) << ": " << scene->getName() << std::endl;
    
    // TODO: lazy load
    //scene->init(this); // TODO: remove dependency on OculonApp
    scene->setup();
    scene->setupInterfaces();
    
    // TODO: autostart?
//    if( autoStart )
//    {
//        scene->setRunning(true);
//        scene->setVisible(true);
//        scene->setDebug(true);
//    }
    mScenes.push_back(scene);
    mScenesByName[scene->getName()] = scene;
    
    // TODO: scene gallery interface
}
