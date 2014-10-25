//
//  CameraController.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2013-03-14.
//
//

#include "CameraController.h"
#include "OculonApp.h"
#include "SceneCam.h"
#include "ManualCam.h"

using namespace ci;
using namespace ci::app;
using namespace std;

CameraController::CameraController()
: mCurrentCameraIndex(0)
{
}

void CameraController::setup(OculonApp* app)
{
    mCameras.push_back( new ManualCam(app->getMayaCam() ) );
    mAspectRatio = app->getViewportAspectRatio();
}

void CameraController::addCamera(SceneCam *sceneCam)
{
    assert(sceneCam);
    mCameras.push_back(sceneCam);
}

void CameraController::setupInterface(Interface *interface, const std::string& name)
{
    interface->gui()->addColumn();
    vector<string> cameraNames = getCameraNames();
    interface->addEnum(CreateEnumParam( "camera", (int*)(&mCurrentCameraIndex) )
                       .maxValue(cameraNames.size())
                       .isVertical()
                       .oscReceiver(name)
                       .sendFeedback(), cameraNames)->registerCallback( this, &CameraController::onCameraChanged );

    for( vector<SceneCam*>::const_reference cam: mCameras )
    {
        cam->setupInterface(interface, name);
    }
    onCameraChanged();
}

const std::vector<std::string> CameraController::getCameraNames()
{
    vector<string> names;
    for( vector<SceneCam*>::const_reference cam: mCameras )
    {
        names.push_back(cam->getName());
    }
    return names;
}

bool CameraController::onCameraChanged()
{
    for( vector<SceneCam*>::const_reference cam: mCameras )
    {
        cam->showInterfacePanel(cam == mCameras[mCurrentCameraIndex]);
    }
    
    return true;
}

void CameraController::update(double dt)
{
    mCameras[mCurrentCameraIndex]->update(dt);
}

const Camera& CameraController::getCamera()
{
    return mCameras[mCurrentCameraIndex]->getCamera();
}
