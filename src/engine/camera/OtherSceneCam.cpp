//
//  OtherSceneCam.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-10-25.
//
//

#include "OtherSceneCam.h"
#include "Scene.h"
#include "OculonApp.h"

OtherSceneCam::OtherSceneCam(OculonApp* app, const std::string& sceneName)
: SceneCam(sceneName, app->getViewportAspectRatio())
, mApp(app)
{
}

const ci::Camera& OtherSceneCam::getCamera()
{
    Scene* scene = mApp->getScene( getName() );
    
    if( scene && scene->isRunning() )
    {
        return scene->getCamera();
    }
    else
    {
        return mCam;
    }
}
