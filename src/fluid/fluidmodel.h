/*
    fluidmodel.h
    Abstract base class for representing a fluid including simulated fluid dynamics
 
    More information about the basic principles, the inner workings and the meaning of the
    interface functions can be found in fluidmodel_cpu.h and fluidmodel_gpu_glsl.h
 
    syfluid - Mirko's interactive fluidum
    done by sy2002 in 2013
  
    contact me at code@sy2002.de
 
    Licensed under CC BY 3.0: http://creativecommons.org/licenses/by/3.0/
    You are free to Share (to copy, distribute and transmit the work),
    to Remix (to adapt the work), to make commercial use of the work
    as long as you credit it to sy2002 and link to http://www.sy2002.de

    Based on Jos Stam's "Real-Time Fluid Dynamics for Games"
    http://www.dgp.toronto.edu/people/stam/reality/Research/pdf/GDC03.pdf
*/

#pragma once

class FluidModel
{
    public:
    
        FluidModel(unsigned int _n, float _viscosity, float _dt, float _diff) :
            N(_n),
            GridSize(N + 2),
            Viscosity(_viscosity),
            DT(_dt),
            Diff(_diff) {;}
    
        inline virtual ~FluidModel() {;}
    
        //reset functions
        virtual void reset() = 0;
        virtual void resetVelocity() = 0;
        virtual void resetDensity() = 0;
        
        //data model modification functions
        virtual void addCircularDensity(unsigned int x, unsigned int y, float s, float r) = 0;
        virtual void addCircularVelocity(unsigned int x, unsigned int y, float sx, float sy, float r) = 0;
        
        //run simulation
        virtual void iterateModel() = 0;    
        virtual void calculateNextIterationForVelocity() = 0;
        virtual void calculateNextIterationForDensity() = 0;
        
        //getters for data model
        virtual const float getDensity(unsigned int x, unsigned int y) const = 0;
        virtual const float getVelocityX(unsigned int x, unsigned int y) const = 0;
        virtual const float getVelocityY(unsigned int x, unsigned int y) const = 0;
    
        //getters for basic simulation data
        inline const unsigned int getQuadraticSize() const {return N;}
        inline const float getViscosity() const {return Viscosity;}
        inline const float getDiffusion() const {return Diff;}
        inline const float getDeltaTime() const {return DT;}
    
        //trivial setter for basic simulation data
        inline void setBasics(float _viscosity, float _diff, float _dt) {Viscosity = _viscosity; Diff = _diff; DT = _dt;}
    
        //non trivial setter: clear all and create a completely new model with the given new quadratic size
        virtual void setQuadraticSize(unsigned int _n) = 0;
    
    protected:
    
        //simulation parameters
        unsigned int                N;              //calculation grid
        unsigned int                GridSize;       //two more grid cells per dimension for boundary management
        float                       Viscosity;      //viscosity of the fluid
        float                       DT;             //change rate (delta-time)
        float                       Diff;           //degree of diffusion of density over time;
};
