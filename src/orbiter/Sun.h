/*
 *  Sun.h
 *  Oculon
 *
 *  Created by Ehsan on 11-11-02.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __SUN_H__
#define __SUN_H__

#include "Body.h"
#include "MoviePlayer.h"

using namespace ci;

class Scene;
class Orbiter;

class Sun : public Body
{
public:
    Sun(Orbiter* scene,
        const Vec3d& pos, 
        const Vec3d& vel, 
        float radius, 
        double rotSpeed,
        double mass, 
        const ColorA& color);
    
    virtual ~Sun();
    
    // inherited from Body
    void setup();
    void update(double dt);
    void draw(const Matrix44d& transform, bool drawBody);
    
private:
    MoviePlayer mMoviePlayer;
    
    static GLfloat mat_ambient[];
    static GLfloat mat_diffuse[];
    static GLfloat mat_emission[];
};

#endif // __SUN_H__
