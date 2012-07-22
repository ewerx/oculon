/*
 ALL CREDIT for this code goes to KYLE MCDONALD and his original OF version
 posted here http://www.openframeworks.cc/forum/viewtopic.php?f=12&t=2860
 */

#pragma once
#include <vector>
#include <cmath>
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

//#define BINNED_QUADS 1

using namespace std;
using namespace ci;

namespace bps {

class Particle {
public:
	float x, y;
	float xv, yv;
	float xf, yf;
	Particle(float _x = 0, float _y = 0,
		float _xv = 0, float _yv = 0) :
		x(_x), y(_y),
		xv(_xv), yv(_yv) {
	}
	void updatePosition(float timeStep) {
		// f = ma, m = 1, f = a, v = int(a)
		xv += xf;
		yv += yf;
		x += xv * timeStep;
		y += yv * timeStep;
	}
	void resetForce() {
		xf = 0;
		yf = 0;
	}
	void bounceOffWalls(float left, float top, float right, float bottom, float damping = .3) 
    {
		bool collision = false;

		if (x > right)
        {
			x = right;
			xv *= -1;
			collision = true;
		} 
        else if (x < left)
        {
			x = left;
			xv *= -1;
			collision = true;
		}

		if (y > bottom)
        {
			y = bottom;
			yv *= -1;
			collision = true;
		} 
        else if (y < top)
        {
			y = top;
			yv *= -1;
			collision = true;
		}

		if (collision == true)
        {
			xv *= damping;
			yv *= damping;
		}
	}
    
    void bounceOffCircularWall(const Vec2f& center, float radius, float radius_sq, float damping = 0.3f)
    {
        const float xsq = (center.x - x) * (center.x - x);
        const float ysq = (center.y - y) * (center.y - y);
        
        if( (xsq + ysq) > (radius_sq) )
        {
            //const float t = Rand::randFloat(2*M_PI);
            const float dx = sqrt( radius_sq - xsq ) * ( (x < center.x) ? -1.0f : 1.0f );
            const float dy = sqrt( radius_sq - ysq ) * ( (y < center.y) ? -1.0f : 1.0f );
            x = center.x + dx;//radius * cos(t);
            y = center.y + dy;//radius * sin(t);
            
            xv *= -1;
            yv *= -1;
            
            xv *= damping;
			yv *= damping;
        }
    }
    
	void addDampingForce(float damping = .01) {
		xf = xf - xv * damping;
        yf = yf - yv * damping;
	}
	void draw(const ci::ColorAf& baseColor, float radius) 
    {
        ci::ColorAf color(baseColor);
        //const float mag = (xf+yf) * (xf+yf);
        //color += ci::ColorAf( mag, mag, mag, 0.0f );
        glColor4f( color.r, color.g, color.b, color.a );
#if BINNED_QUADS
        //gl::drawBillboard(Vec2f(x,y), Vec2f(radius,radius), 0.0f, bbRight, bbUp);
        glTexCoord2f( 0, 0 );
        glVertex2f( x - radius, y - radius );
        
        glTexCoord2f( 1, 0 );
        glVertex2f( x + radius, y - radius );
        
        glTexCoord2f( 1, 1 );
        glVertex2f( x + radius, y + radius );
        
        glTexCoord2f( 0, 1 );
        glVertex2f( x - radius, y + radius );
#else
		glVertex2f(x, y);
#endif
	}
};

}