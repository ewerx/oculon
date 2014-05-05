//
//  Geometry.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-04.
//
//

#pragma once

#include "AudioInputHandler.h"
#include "CameraController.h"
#include "EaseCurveSelector.h"
#include "Entity.h"
#include "Scene.h"

#include "cinder/Cinder.h"
#include <vector>

//
// Geometry
//
class Geometry : public Scene
{
public:
    Geometry();
    virtual ~Geometry();
    
    // inherited from Scene
    void setup();
    void shutdown();
    void reset();
    void update(double dt);
    void draw();
//    void drawDebug();
    const ci::Camera& getCamera();

protected:// from Scene
    void setupInterface();

private:
    std::vector<Entityf*> mObjects;
    
    // camera
    CameraController mCameraController;
    
    // params
    bool mDrawWireframe;
};