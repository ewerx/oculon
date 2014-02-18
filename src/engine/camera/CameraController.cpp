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
#include "cinder/Quaternion.h"

using namespace ci;
using namespace std;

CameraController::CameraController()
: mCamType(CAM_MANUAL)
{
    for (int i = 0; i < CAM_COUNT; ++i)
    {
        mInterfacePanels[i] = NULL;
    }
}

void CameraController::setup(OculonApp *app, const unsigned int camTypes, eCamType defaultCam)
{
    mApp = app;
    mAvailableCamTypes = camTypes;
    mCamType = defaultCam;
    mSplineCam.setup(5000.0f,500.0f);
    mStarCam.setup(mApp);
    mSpringCam = SpringCam( -420.0f, mApp->getViewportAspectRatio(), 3000.0f );
    
    mSpinCam.lookAt(Vec3f(0.0f,0.0f,-420.0f), Vec3f(0.0f,0.0f,0.0f), Vec3f(0.0f,1.0f,0.0f));
    mSpinRate = 0.1f;
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
                        .isVertical(), camTypeNames)->registerCallback( this, &CameraController::onCameraChanged );
    
    mInterfacePanels[CAM_SPLINE] = interface->gui()->addPanel();
    mSplineCam.setupInterface(interface, sceneName);
    
    mInterfacePanels[CAM_STAR] = interface->gui()->addPanel();
    mStarCam.setupInterface(interface, sceneName);
    
    mInterfacePanels[CAM_SPIN] = interface->gui()->addPanel();
    interface->gui()->addLabel("spin cam");
    interface->addParam(CreateFloatParam("spin_rate", &mSpinRate));
    
    onCameraChanged();
}

bool CameraController::onCameraChanged()
{
    for (int i = 0; i < CAM_COUNT; ++i)
    {
        if (mInterfacePanels[i] && i != mCamType)
        {
            mInterfacePanels[i]->enabled = false;
        }
    }
    
    if (mInterfacePanels[mCamType]) {
        mInterfacePanels[mCamType]->enabled = true;
    }
    
    return true;
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
            
        case CAM_STAR:
            mStarCam.update(dt);
            
        case CAM_SPIN:
            mSpinQuat *= Quatf(Vec3f(0.0f,1.0f,0.0f), dt*mSpinRate);
            //mSpinCam.setOrientation(mSpinQuat);
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
            
        case CAM_CATALOG:
        {
            Scene* orbiterScene = mApp->getScene("parsec");
            
            if( orbiterScene && orbiterScene->isRunning() )
            {
                return orbiterScene->getCamera();
            }
            else
            {
                return mStarCam.getCamera();
            }
        }
            break;
            
        case CAM_SPIN:
            return mSpinCam;
            
        case CAM_MANUAL:
        default:
            return mApp->getMayaCam();
    }
}
