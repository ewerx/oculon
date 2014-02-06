//
//  CameraController.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2013-03-14.
//
//

#include "CameraController.h"
#include "OculonApp.h"
#include "Scene.h"

using namespace ci;
using namespace std;

CameraController::CameraController()
: mCamType(CAM_MANUAL)
{
}

void CameraController::setup(OculonApp *app, const unsigned int camTypes, eCamType defaultCam)
{
    mApp = app;
    mAvailableCamTypes = camTypes;
    mCamType = defaultCam;
    mSplineCam.setup();
    mStarCam.setup(mApp);
    mSpringCam = SpringCam( -420.0f, mApp->getViewportAspectRatio(), 3000.0f );
}

void CameraController::setupInterface(Interface *interface, const std::string& sceneName)
{
    interface->gui()->addColumn();
    vector<string> camTypeNames;
    int camCount = 0;
#define CAMCTRLR_CAMTYPE_ENTRY( nam, enm, val ) \
        camTypeNames.push_back(nam);
    CAMCTRLR_CAMTYPE_TUPLE
#undef  CAMCTRLR_CAMTYPE_ENTRY
    interface->addEnum(CreateEnumParam( "camera", (int*)(&mCamType), sceneName )
                        .maxValue(camCount)
                        .isVertical(), camTypeNames);
    
    mSplineCam.setupInterface(interface, sceneName);
}

void CameraController::update(double dt)
{
    switch( mCamType )
    {
            //TODO: spring cam needs mouse controls
            //TODO: move maya cam here too?
        case CAM_SPRING:
            mSpringCam.update(0.3f);
            break;
            
        case CAM_SPLINE:
            mSplineCam.update(dt);
            break;
            
        default:
            break;
    }
}

const Camera& CameraController::getCamera()
{
    switch( mCamType )
    {
        case CAM_SPRING:
            return mSpringCam.getCam();
            
        case CAM_SPLINE:
            return mSplineCam.getCamera();
            
        case CAM_STAR:
            return mStarCam.getCamera();
            
        case CAM_GRAVITON:
        {
            Scene* gravitonScene = mApp->getScene("graviton");
            
            if( gravitonScene && gravitonScene->isRunning() )
            {
                return gravitonScene->getCamera();
            }
            else
            {
                return mSpringCam.getCam();
            }
        }
            break;
            
        case CAM_ORBITER:
        {
            Scene* orbiterScene = mApp->getScene("orbiter");
            
            if( orbiterScene && orbiterScene->isRunning() )
            {
                return orbiterScene->getCamera();
            }
            else
            {
                return mSpringCam.getCam();
            }
        }
            break;
            
        case CAM_MANUAL:
        default:
            return mApp->getMayaCam();
    }
}
