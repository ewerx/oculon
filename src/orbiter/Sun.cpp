/*
 *  Sun.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-11-02.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Sun.h"
#include "Orbiter.h"
#include "Resources.h"
#include "OculonApp.h"
#include "cinder/gl/gl.h"

using namespace ci;

GLfloat Sun::mat_ambient[]		= { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat Sun::mat_diffuse[]		= { 0.8f, 0.8f, 0.8f, 1.0f };
GLfloat Sun::mat_emission[]     = { 1.0f, 1.0f, 1.0f, 1.0f };


Sun::Sun(Scene* scene,
         const Vec3d& pos, 
         const Vec3d& vel, 
         float radius, 
         double rotSpeed,
         double mass, 
         const ColorA& color) 
: Body(scene,"Sol",pos,vel,radius,rotSpeed,mass,color,loadImage(loadResource(RES_ORBITER_SUN)))
, mMoviePlayer(scene)
{
}

Sun::~Sun()
{
}

void Sun::setup()
{
    mMoviePlayer.setup();
    //mMoviePlayer.loadMoviePrompt();
}

void Sun::update(double dt)
{
    Body::update(dt);
    
    mMoviePlayer.update(dt);
}

void Sun::draw(const Matrix44d& transform, bool drawBody)
{
    static const int sphereDetail = 64;
    
    if( drawBody )
    {
        Vec3d screenCoords = transform * mPosition;
        float radius = mRadius*mRadiusMultiplier;
        glPushMatrix();
        //glEnable( GL_LIGHTING );
        
        glTranslatef(screenCoords.x, screenCoords.y, screenCoords.z);
        
        //drawDebugVectors();
        
        
        
        gl::Texture texture;
        if( mMoviePlayer.isPlaying() )
        {
            glMaterialfv( GL_FRONT, GL_AMBIENT,	Body::no_mat );
            glMaterialfv( GL_FRONT, GL_SPECULAR, Body::no_mat );
            glMaterialfv( GL_FRONT, GL_SHININESS, Body::no_shininess );
            glMaterialfv( GL_FRONT, GL_EMISSION, Body::mat_emission );
            glMaterialfv( GL_FRONT, GL_DIFFUSE,	ColorA(1.0f, 1.0f, 1.0f) );
            
            texture = mMoviePlayer.getMovie().getTexture();
            //glColor4f(1.0f,1.0f,1.0f,1.0f);
        }
        else
        {
            //glMaterialfv( GL_FRONT, GL_AMBIENT,	Orbiter::mat_ambient );
            glMaterialfv( GL_FRONT, GL_AMBIENT,	Body::no_mat );
            glMaterialfv( GL_FRONT, GL_SPECULAR, Body::no_mat );
            glMaterialfv( GL_FRONT, GL_SHININESS, Body::no_shininess );
            glMaterialfv( GL_FRONT, GL_EMISSION, Sun::mat_emission );
            
            texture = mTexture;
            texture.setWrap( GL_REPEAT, GL_REPEAT );
            //mTexture.enableAndBind();
        }
        
        texture.bind();
        //glMaterialfv( GL_FRONT, GL_DIFFUSE,	mColor );
        //glColor4f( mColor );
        gl::drawSphere( Vec3d::zero(), radius, sphereDetail );
        
        texture.unbind();
        
        if( Orbiter::sDrawRealSun )
        {
            double sunRadius = 695500000.0f * Orbiter::sDrawScale * 500000.0f;
            sunRadius = sunRadius * mRadiusMultiplier * 0.75f;
            gl::enableWireframe();
            glColor4f(1.0f,0.0f,0.0f,1.0f);
            gl::drawSphere( Vec3d::zero(), sunRadius, 16 );
            gl::disableWireframe();
        }
        
        const bool binned = true;
        if( binned )
        {
            //TODO: hack, use a message
            Binned* binnedScene = NULL;//static_cast<Binned*>(app->getScene("binned"));
            
            if( binnedScene && binnedScene->isRunning() )
            {
                Vec3d screenCoords = transform * mPosition;
                Vec2f textCoords = mParentScene->getCamera().worldToScreen(screenCoords, mParentScene->getApp()->getViewportWidth(), mParentScene->getApp()->getViewportHeight());
                float force = 500.f;
                binnedScene->addRepulsionForce(textCoords, mRadius*mRadiusMultiplier*0.5f, force*mRadiusMultiplier);
            }
        }
        
        glPopMatrix();
        
        //mMoviePlayer.draw();
    }
}
