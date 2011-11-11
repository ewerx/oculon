/*
 *  Sun.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-11-02.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Sun.h"

Sun::Sun(const Vec3d& pos, 
         const Vec3d& vel, 
         float radius, 
         double mass, 
         const ColorA& color) 
: Body(pos,vel,radius,mass,color)
{
}

Sun::~Sun()
{
}

void Sun::draw(const Matrix44d& transform)
{
    Body::draw(transform);
}
