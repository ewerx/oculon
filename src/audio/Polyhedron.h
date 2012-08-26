/*
 *  Polyhedron.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#pragma once

#include "cinder/TriMesh.h"
#include "cinder/gl/Vbo.h"

class Scene;

//TODO: refactor as SubScene
class Polyhedron
{
public:
    Polyhedron(Scene* scene);
    virtual ~Polyhedron();

    void setup();
    void setupInterface();
    void setupDebugInterface();
    void resize();
    void reset();
    void update(double dt);
    void draw();
    
private:
    ci::TriMesh			mMesh;
    ci::gl::VboMesh		mVBO;
    
    Scene*  mParentScene;

};
