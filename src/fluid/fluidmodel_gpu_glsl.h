/*
    fluidmodel_gpu_glsl.h
    Representation of a fluid including simulated fluid dynamics
 
    syfluid - Mirko's interactive fluidum
    done by sy2002 in 2011 and 2013
 
    GLSL GPU version
 
    Quadratic matrix representation of fluid modelled by
    a scalar density field. Fluid dynamics are modelled
    by approximate linear solvers of the Navier-Stokes Equations.
    The fluid dynamics are performed on a constant time-step
    base by repetitve calls to the simulation functions.
 
    contact me at code@sy2002.de
 
    Licensed under CC BY 3.0: http://creativecommons.org/licenses/by/3.0/
    You are free to Share (to copy, distribute and transmit the work),
    to Remix (to adapt the work), to make commercial use of the work
    as long as you credit it to sy2002 and link to http://www.sy2002.de
 
    Based on Jos Stam's "Real-Time Fluid Dynamics for Games"
    http://www.dgp.toronto.edu/people/stam/reality/Research/pdf/GDC03.pdf
*/

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/area.h"
#include "cinder/gl/fbo.h"
#include "cinder/gl/glslprog.h"
#include "cinder/rect.h"
#include "cinder/vector.h"
#include "cinder/Surface.h"

#include "fluidmodel.h"

class FluidModelGLSL: public FluidModel
{
    public:

        //create new FluidModel, reserve memory for fluid model's shared_ptrs ("theapp" is needed for resource and console access)
        FluidModelGLSL(unsigned int _n, float _viscosity, float _dt, float _diff, ci::app::AppBasic* theapp);
        virtual inline ~FluidModelGLSL() {;}
        
        //non trivial setter: clear all and create a completely new model with the given new quadratic size
        virtual void setQuadraticSize(unsigned int _n);
    
        //reset the state of the simulation (construction does this automatically, so only use it, when really needed)
        virtual void reset();           //reset the whole model
        virtual void resetVelocity();   //reset only the velocity model
        virtual void resetDensity();    //reset only the density model
    
        //modify the running simulation
        virtual void addCircularDensity(unsigned int x, unsigned int y, float s, float r);  //center (x, y), strength s, radius r
        virtual void addCircularVelocity(unsigned int x, unsigned int y, float sx, float sy, float r); //dito, strength is a vector (sx, sy)
    
        //an iteration takes the previous state as an input parameter that is "+= DT * value" added to the
        //current state (meaning DT is reducing the impact); this means, that you can modify the running
        //simulation *before* calling iterateModel(), because the previous state is implicitly deleted after the
        //iteration to make room for either further changes or for a continuation of the simulation using the current state
        virtual void iterateModel();
    
        //manual iteration of the high-level fluid simulation functions: calculate next step
        //iterateModel() is preferably called instead, because it respects the calling order (velocity first)
        //and because it clears the previous state afterwards; so only use those two functions if you
        //know what you're doing
        virtual void calculateNextIterationForVelocity();
        virtual void calculateNextIterationForDensity();
    
        //access fluid model using (x,y) coordinates ranging from (1..N)
        //not implemented for GPU mode, because this is way too slow
        virtual inline const float getDensity(unsigned int x, unsigned int y) const {return 0;}
        virtual inline const float getVelocityX(unsigned int x, unsigned int y) const {return 0;}
        virtual inline const float getVelocityY(unsigned int x, unsigned int y) const {return 0;}
    
        //access fluid model's internal representation as a texture for avoiding costly copy operations from VRAM to RAM
        virtual inline ci::gl::Texture& getDensity() {return Density.getTexture();}
    
        //when accessing the model for drawing, we should run from 1..N instead of 0..GridSize
        virtual inline ci::Rectf getDensityTextureCoordinates()
        {
            ci::Area texturearea = Density.getTexture().getCleanBounds();
            ++texturearea.x1; ++texturearea.y1;
            --texturearea.x2; --texturearea.y2;
            ci::Rectf retval = Density.getTexture().getAreaTexCoords(texturearea);
            return retval;
        }

    
    protected:
    
        typedef ci::gl::Fbo         GPUBuffer;
        typedef ci::gl::GlslProg    GPUShader;
        
        void initFBOs();            //create all FBOs from the basic parameters and the quadratic size
    
        //initialize a whole field with a given value
        void initField(GPUBuffer& field, float value = 0.0f);
    
        //put circular "pixel fields", centered at (x, y) with radius r and "strength" s (s reduces the radial force decay)
        void addCircularForceArea(GPUBuffer& field, int x, int y, float s, float r);
    
        //core functions of the linear approximative Navier-Stokes Equation solver
        //see Jos Stam's paper for detailed descriptions
        void setBnd(GPUBuffer& x, int b);
        void linSolve(GPUBuffer& x, GPUBuffer& x0, int b, float a, float c);
        void addSource(GPUBuffer& x, GPUBuffer& s);
        void diffuse(GPUBuffer& x, GPUBuffer& x0, int b, float actual_diffuse);
        void advect(GPUBuffer& d, GPUBuffer& d0, int b, GPUBuffer& _u, GPUBuffer& _v);
        void project(GPUBuffer& _u, GPUBuffer& _v, GPUBuffer& _p, GPUBuffer& _div);
    
        //reference to cinder application for being able to load resources and to log to the console
        ci::app::AppBasic*          TheApp;    
    
        //additional simulation parameters
        float                       OneNStep;       //one pixel in the GPUBuffers in float coordinates
        
        //perform calculations according to previously setup actions (aka shaders) and variables and data buffers (aka uniform values and textures)
        void renderToGPUBuffer(GPUBuffer& x, ci::Area* texturesource = NULL);
    
        //shaders (calculation units which are doing the actual job) named according to Jos Stam's core functions
        GPUShader                   SetBndShader;
        GPUShader                   AddSourceShader;
        GPUShader                   LinSolveShader;
        GPUShader                   AddVectShader;
        GPUShader                   ProjectShader_Step1;
        GPUShader                   ProjectShader_Step2;
        GPUShader                   ProjectShader_Step3;
        GPUShader                   AddForceShader_Circular;
    
        //fluid dynamics are modelled via a force vector-field and a density scalar-field
        GPUBuffer                   u;              //x-component of force vector-field
        GPUBuffer                   v;              //y-component of force vector-field
        GPUBuffer                   u_prev;         //previous iteration of u
        GPUBuffer                   v_prev;         //previous iteration of v
        GPUBuffer                   Density;        //fluid's density scalar-field
        GPUBuffer                   Density_prev;   //previous iteration of Density
    
        //sampling textures and writing to them in the same time is not allowed in GLSL (e.g. x[i] += s[i] is not allowed and needs a buffer)
        GPUBuffer                   WorkBuffer;    
    
        //RAM buffers (aka Surfaces) for returning single pixels
        ci::Surface32f              uSurface;
        ci::Surface32f              vSurface;
        ci::Surface32f              DensitySurface;
};
