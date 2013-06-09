//
//  CameraController.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2013-03-14.
//
//

#ifndef __Oculon__CameraController__
#define __Oculon__CameraController__

#include "SplineCam.h"
#include "SpringCam.h"
#include "StarCam.h"
#include "cinder/Camera.h"

class OculonApp;

class CameraController
{
public:
    
    // camera
#define CAMCTRLR_CAMTYPE_TUPLE \
CAMCTRLR_CAMTYPE_ENTRY( "Manual",   CAM_MANUAL,     (1u << 0) ) \
CAMCTRLR_CAMTYPE_ENTRY( "Spline",   CAM_SPLINE,     (1u << 1) ) \
CAMCTRLR_CAMTYPE_ENTRY( "Spring",   CAM_SPRING,     (1u << 2) ) \
CAMCTRLR_CAMTYPE_ENTRY( "Star",     CAM_STAR,       (1u << 3) ) \
CAMCTRLR_CAMTYPE_ENTRY( "Orbiter",  CAM_ORBITER,    (1u << 4) ) \
CAMCTRLR_CAMTYPE_ENTRY( "Graviton", CAM_GRAVITON,   (1u << 5) ) \
CAMCTRLR_CAMTYPE_ENTRY( "Catalog",  CAM_CATALOG,    (1u << 6) ) \
//end tuple
    
    enum eCamType
    {
#define CAMCTRLR_CAMTYPE_ENTRY( nam, enm, val ) \
enm = val,
        CAMCTRLR_CAMTYPE_TUPLE
#undef  CAMCTRLR_CAMTYPE_ENTRY
        
        CAM_COUNT
    };
    
public:
    CameraController(OculonApp *app, const unsigned int camTypes);
    virtual ~CameraController() {}
    
    void update(double dt);
    
    eCamType getCamType() const                 { return mCamType; }
    void setCamType( const eCamType camType )   { mCamType = camType; }
    const ci::Camera& getCamera();
    
protected:
    OculonApp*                  mApp;
    eCamType                    mCamType;
    unsigned int                mAvailableCamTypes;
    
    SplineCam                   mSplineCam;
    SpringCam                   mSpringCam;
    StarCam                     mStarCam;
};

#endif /* defined(__Oculon__CameraController__) */
