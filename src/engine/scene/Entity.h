/*
 *  Entity.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-17.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "cinder/Cinder.h"
#include "cinder/Vector.h"

using namespace ci;

class Scene;

template<typename T>
class Entity
{
public:
    Entity(Scene* scene);
    Entity(Scene* scene, const Vec3<T>& pos);
    virtual ~Entity();
    
    virtual void setup() {}
    virtual void update(double dt) {}
    virtual void draw() {}
    
    const Vec3<T>& getPosition() const      { return mPosition; }
    void setPosition(const Vec3<T>& val)    { mPosition = val; }
    
protected:
    Vec3<T> mPosition;
    
    Scene*  mParentScene;
};

typedef Entity<float> Entityf;
typedef Entity<double> Entityd; 

#endif // __ENTITY_H__
