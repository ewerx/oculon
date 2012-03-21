/*
 *  Entity.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-17.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Entity.h"
#include "Scene.h"

using namespace ci;

// constructor
//
template<typename T>
Entity<T>::Entity( Scene* scene )
: mParentScene(scene)
, mPosition(Vec3<T>::zero())
{
}

// constructor
//
template<typename T>
Entity<T>::Entity( Scene* scene, const Vec3<T>& pos )
: mParentScene(scene)
, mPosition(pos)
{
}

// destructor
//
template<typename T>
Entity<T>::~Entity()
{
}

// to allow derived classes to call the Entity constructor from their initialization list
template class Entity<float>;
template class Entity<double>;