/*
 *  Sun.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-11-02.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Sun.h"

using namespace ci;

GLfloat Sun::mat_ambient[]		= { 0.5, 0.5, 0.5, 1.0 };
GLfloat Sun::mat_diffuse[]		= { 0.8, 0.8, 0.8, 1.0 };
GLfloat Sun::mat_emission[]     = { 0.8, 0.8, 0.8, 0.0 };


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

void Sun::draw(const Matrix44d& transform, bool drawBody)
{
    static const int sphereDetail = 64;
    Vec3d screenCoords = transform * mPosition;
    
    if( drawBody )
    {
        glPushMatrix();
        //glEnable( GL_LIGHTING );
        
        //glMaterialfv( GL_FRONT, GL_AMBIENT,	Orbiter::mat_ambient );
        glMaterialfv( GL_FRONT, GL_AMBIENT,	Body::no_mat );
        glMaterialfv( GL_FRONT, GL_SPECULAR, Body::no_mat );
        glMaterialfv( GL_FRONT, GL_SHININESS, Body::no_shininess );
        glMaterialfv( GL_FRONT, GL_EMISSION, Sun::mat_emission );
        
        
        glTranslatef(screenCoords.x, screenCoords.y, screenCoords.z);
        
        glMaterialfv( GL_FRONT, GL_DIFFUSE,	mColor );
        //glColor4f( mColor.r, mColor.g, mColor.b, mColor.a );
        gl::drawSphere( Vec3d::zero(), mRadius*mRadiusMultiplier/2.0f, sphereDetail );
        
        glPopMatrix();
    }
}
