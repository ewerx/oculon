//
//  PlanetScene.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-26.
//
//

#pragma once

#include "Scene.h"
#include "PlanetObject.h"

class PlanetScene : public Scene
{
public:
    PlanetScene();
    virtual ~PlanetScene();
    
    // inherited from Scene
    void setup();
    void reset();
    //    void resize();
    void update(double dt);
    void draw();
    const ci::Camera& getCamera();
    
protected:
    // inherited from Scene
    void setupInterface();
    
private:
    PlanetObject mPlanet;
    
    CameraController mCameraController;
};