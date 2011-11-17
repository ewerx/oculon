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
: Body("Sol",pos,vel,radius,mass,color)
{
}

Sun::~Sun()
{
}

void Sun::draw(const Matrix44d& transform, bool drawBody)
{
    static const int sphereDetail = 64;
    
    if( drawBody )
    {
        Vec3d screenCoords = transform * mPosition;
        float radius = mRadius*mRadiusMultiplier*0.75f;
        glPushMatrix();
        //glEnable( GL_LIGHTING );
        
        glTranslatef(screenCoords.x, screenCoords.y, screenCoords.z);
        
        //drawDebugVectors();
        
        //glMaterialfv( GL_FRONT, GL_AMBIENT,	Orbiter::mat_ambient );
        glMaterialfv( GL_FRONT, GL_AMBIENT,	Body::no_mat );
        glMaterialfv( GL_FRONT, GL_SPECULAR, Body::no_mat );
        glMaterialfv( GL_FRONT, GL_SHININESS, Body::no_shininess );
        glMaterialfv( GL_FRONT, GL_EMISSION, Sun::mat_emission );
        
        //glMaterialfv( GL_FRONT, GL_DIFFUSE,	mColor );
        //glColor4f( mColor );
        gl::drawSphere( Vec3d::zero(), radius, sphereDetail );
        
        glPopMatrix();
    }
}
