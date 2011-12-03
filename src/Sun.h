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

using namespace ci;

class Sun : public Body
{
public:
    Sun(const Vec3d& pos, 
        const Vec3d& vel, 
        float radius, 
        double mass, 
        const ColorA& color);
    
    virtual ~Sun();
    
    // inherited from Body
    //void update(double dt);
    void draw(const Matrix44d& transform, bool drawBody);
    
private:
    static GLfloat mat_ambient[];
    static GLfloat mat_diffuse[];
    static GLfloat mat_emission[];
};

#endif // __SUN_H__
