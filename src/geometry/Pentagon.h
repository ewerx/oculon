//
//  Pentagon.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-04.
//
//

#pragma once

#include "Entity.h"
#include "cinder/gl/Vbo.h"


class Pentagon : public Entity<float>
{
public:
    Pentagon(Scene* scene, float radius);
    ~Pentagon() {}
  
    // inherited from Entity
    void setup();
    void update(double dt);
    void draw();
    
private:
    float mRadius;
    
    ci::gl::VboMesh mMesh;
};