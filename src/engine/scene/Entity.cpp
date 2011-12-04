/*
 *  Entity.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-17.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Entity.h"

using namespace ci;

// constructor
//
template<typename T>
Entity<T>::Entity()
: mPosition(Vec3<T>::zero())
{
}

// constructor
//
template<typename T>
Entity<T>::Entity( const Vec3<T>& pos )
: mPosition(pos)
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