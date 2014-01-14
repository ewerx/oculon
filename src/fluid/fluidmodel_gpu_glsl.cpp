/*
    fluidmodel_gpu_glsl.cpp
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

#include <math.h>
#include <iostream>

#include "cinder/gl/fbo.h"
#include "cinder/gl/texture.h"
#include "cinder/surface.h"
#include "cinder/Utilities.h"

#include "convenience.h"
#include "fluidmodel_gpu_glsl.h"
#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace ci::gl;

FluidModelGLSL::FluidModelGLSL(const unsigned int _n, const float _viscosity, const float _dt, const float _diff, AppBasic* theapp) :
    FluidModel(_n, _viscosity, _dt, _diff),    
    TheApp(theapp),
    //during the calculations, we are running through the whole buffer, i.e. 0..GridSize; this contrasts
    //the drawing (running from 1..N) which is occuring out of the FluidModelGLSL, e.g. in the main app
    OneNStep(1.0f / ((float) (_n + 2) - 1.0f)) 
{
    //create off-screen Frame Buffer Objects in VRAM
    initFBOs();
    
    //load and compile GLSL code
	try
    {
        SetBndShader = GlslProg(TheApp->loadResource(RES_PASSTHRU_VERT), TheApp->loadResource(RES_GLSL_FRAG_SETBND));
        AddSourceShader = GlslProg(TheApp->loadResource(RES_PASSTHRU_VERT), TheApp->loadResource(RES_GLSL_FRAG_ADDSOURCE));
        LinSolveShader = GlslProg(TheApp->loadResource(RES_PASSTHRU_VERT), TheApp->loadResource(RES_GLSL_FRAG_LINSOLVE));
        AddVectShader = GlslProg(TheApp->loadResource(RES_PASSTHRU_VERT), TheApp->loadResource(RES_GLSL_FRAG_ADDVECT));
        ProjectShader_Step1 = GlslProg(TheApp->loadResource(RES_PASSTHRU_VERT), TheApp->loadResource(RES_GLSL_FRAG_PROJECT1));
        ProjectShader_Step2 = GlslProg(TheApp->loadResource(RES_PASSTHRU_VERT), TheApp->loadResource(RES_GLSL_FRAG_PROJECT2));
        ProjectShader_Step3 = GlslProg(TheApp->loadResource(RES_PASSTHRU_VERT), TheApp->loadResource(RES_GLSL_FRAG_PROJECT3));
        AddForceShader_Circular = GlslProg(TheApp->loadResource(RES_PASSTHRU_VERT), TheApp->loadResource(RES_GLSL_FRAG_ADDFORCE_CIRC));
	}
	catch(gl::GlslProgCompileExc &exc)
    {
        TheApp->console() << "Shader compile error: " << std::endl;
        TheApp->console() << exc.what() << std::endl;
	}
	catch(...)
    {
		TheApp->console() << "Unable to load shader." << std::endl;
	}
}

void FluidModelGLSL::initFBOs()
{
    //create off-screen Frame Buffer Objects in VRAM
    //we need GL_RGBA32F for representing the density as preceise floats
    Fbo::Format format;
	format.enableDepthBuffer(false); 
    format.setColorInternalFormat(GL_RGBA32F);
    
    u = Fbo(GridSize, GridSize, format);
    v = Fbo(GridSize, GridSize, format);
    u_prev = Fbo(GridSize, GridSize, format);
    v_prev = Fbo(GridSize, GridSize, format);
    Density = Fbo(GridSize, GridSize, format);
    Density_prev = Fbo(GridSize, GridSize, format);
    WorkBuffer = Fbo(GridSize, GridSize, format);
    
    //initialize all FBOs
    reset();
}

void FluidModelGLSL::setQuadraticSize(unsigned int _n)
{
    //set new values and re-init FBOs
    N = _n;
    GridSize = N + 2;
    OneNStep = 1.0f / ((float) GridSize - 1.0f);
    
    //reallocate FBOs based on the new values
    //since Cinder uses shared_ptrs, assigning a new Fbo (done in initFBOs();) will
    //automatically free the old FBOs in the GPU's memory, so that GPU RAM leaks are occuring
    initFBOs();
}

void FluidModelGLSL::initField(GPUBuffer& field, const float value)
{
    field.bindFramebuffer();
    gl::clear(ColorA(value, value, value, 0.0f));  
    field.unbindFramebuffer();
}

void FluidModelGLSL::resetVelocity()
{
    initField(u);
    initField(v);
    initField(u_prev);
    initField(v_prev);
}

void FluidModelGLSL::resetDensity()
{
    initField(Density);
    initField(Density_prev);
}

void FluidModelGLSL::reset()
{
    resetVelocity();
    resetDensity();
}

void FluidModelGLSL::renderToGPUBuffer(GPUBuffer& x, Area* texturesource)
{
    //adjust coordinate system and camera for 2D FBO rendering
	gl::setMatricesWindow(x.getTexture().getSize(), false);
	gl::setViewport(x.getTexture().getCleanBounds());    
    
    //draw a rectangle (aka "all data from the data source") using two triangles
    x.bindFramebuffer();
    
    Area srcArea = (texturesource == NULL) ? Area(x.getTexture().getCleanBounds()) : *texturesource;
    Rectf destRect = x.getTexture().getCleanBounds();
    
    glEnableClientState(GL_VERTEX_ARRAY);
    GLfloat verts[8];
    glVertexPointer(2, GL_FLOAT, 0, verts);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    GLfloat texCoords[8];
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);  

    verts[0*2+0] = destRect.getX2(); verts[0*2+1] = destRect.getY1();	
    verts[1*2+0] = destRect.getX1(); verts[1*2+1] = destRect.getY1();	
    verts[2*2+0] = destRect.getX2(); verts[2*2+1] = destRect.getY2();	
    verts[3*2+0] = destRect.getX1(); verts[3*2+1] = destRect.getY2();	
    
    const Rectf srcCoords = x.getTexture().getAreaTexCoords(srcArea);
    texCoords[0*2+0] = srcCoords.getX2(); texCoords[0*2+1] = srcCoords.getY1();	
    texCoords[1*2+0] = srcCoords.getX1(); texCoords[1*2+1] = srcCoords.getY1();	
    texCoords[2*2+0] = srcCoords.getX2(); texCoords[2*2+1] = srcCoords.getY2();	
    texCoords[3*2+0] = srcCoords.getX1(); texCoords[3*2+1] = srcCoords.getY2();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);	
    
    x.unbindFramebuffer();
}

void FluidModelGLSL::setBnd(GPUBuffer& x, int b)
{ 
    //Texture unit definition
    const int TexU_x = 0;
    
    //copy x to WorkBuffer since sampling and rendering to the same texture is not allowed/undefined/driver dependend in GLSL
    x.blitTo(WorkBuffer, x.getBounds(), WorkBuffer.getBounds());
    
    //Bind shader and pass variables as uniforms
    SetBndShader.bind();
    SetBndShader.uniform("x", TexU_x);
    SetBndShader.uniform("b", b);
    SetBndShader.uniform("OneNStep", OneNStep);
    
    //Bind textures of FBOs to the above-mentioned texture units
    WorkBuffer.getTexture().bind(TexU_x);
    
    //setBnd(x, b);
    renderToGPUBuffer(x);
    
    //clean up
    WorkBuffer.getTexture().unbind();
    SetBndShader.unbind();
}

void FluidModelGLSL::linSolve(GPUBuffer& x, GPUBuffer& x0, int b, float a, float c)
{   
    //Texture unit definitions
    const int TexU_x = 0;
    const int TexU_x0 = 1;    
    
    for (int k = 0; k < 20; k++)
    {
        //copy x to WorkBuffer since sampling and rendering to the same texture is not allowed/undefined/driver dependend in GLSL
        x.blitTo(WorkBuffer, x.getBounds(), WorkBuffer.getBounds());
        
        //Bind shader and pass variables as uniforms
        LinSolveShader.bind();
        LinSolveShader.uniform("x", TexU_x);
        LinSolveShader.uniform("x0", TexU_x0);
        LinSolveShader.uniform("a", a);
        LinSolveShader.uniform("c", c);
        LinSolveShader.uniform("OneNStep", OneNStep);
        LinSolveShader.uniform("OneMinusOneNStep", 1.0f - OneNStep);
        
        //Bind textures of FBOs to the above-mentioned texture units
        WorkBuffer.getTexture().bind(TexU_x);
        x0.getTexture().bind(TexU_x0);
        
        //for all (i, j) from 1..N:
        //x[IX(i, j)] = (x0[IX(i, j)] +  a * (x[IX(i - 1, j)] + x[IX(i + 1, j)] + x[IX(i, j - 1)] + x[IX(i, j + 1)])) / c; 
        renderToGPUBuffer(x);
                
        //clean up
        WorkBuffer.getTexture().unbind();
        x0.getTexture().unbind();
        LinSolveShader.unbind();

        //manage boundaries of the model matrix
        setBnd(x, b);
    }
}

void FluidModelGLSL::addSource(GPUBuffer& x, GPUBuffer& s)
{
    //copy x to WorkBuffer since sampling and rendering to the same texture is not allowed/undefined/driver dependend in GLSL
    x.blitTo(WorkBuffer, x.getBounds(), WorkBuffer.getBounds());
     
    //Texture unit definitions
    const int TexU_x = 0;
    const int TexU_s = 1;
    
    //Bind shader and pass variables as uniforms
    AddSourceShader.bind();
    AddSourceShader.uniform("x", TexU_x);
    AddSourceShader.uniform("s", TexU_s);
    AddSourceShader.uniform("dt", DT);
    
    //Bind textures of FBOs to the above-mentioned texture units
    WorkBuffer.getTexture().bind(TexU_x);
    s.getTexture().bind(TexU_s);
    
    //perform: x[i] += dt * s[i]
    renderToGPUBuffer(x);
    
    //clean up
    WorkBuffer.getTexture().unbind();
    s.getTexture().unbind();
    AddSourceShader.unbind();
}

void FluidModelGLSL::diffuse(GPUBuffer& x, GPUBuffer& x0, int b, float actual_diffuse)
{
	float a = DT * actual_diffuse * N * N;
    linSolve(x, x0, b, a, 1 + 4 * a);
}

void FluidModelGLSL::advect(GPUBuffer& d, GPUBuffer& d0, int b, GPUBuffer& _u, GPUBuffer& _v)
{
    //Texture unit definitions
    const int TexU_d0 = 0;
    const int TexU_u = 1;
    const int TexU_v = 2;
    
    //Bind shader and pass variables as uniforms
    AddVectShader.bind();
    AddVectShader.uniform("d0", TexU_d0);
    AddVectShader.uniform("u", TexU_u);
    AddVectShader.uniform("v", TexU_v);
    AddVectShader.uniform("OneNStep", OneNStep);
    AddVectShader.uniform("OneMinusOneNStep", 1.0f - OneNStep);
    AddVectShader.uniform("DT0", (1.0f - OneNStep) * DT);

    //Bind textures of FBOs to the above-mentioned texture units
    d0.getTexture().bind(TexU_d0);
    _u.getTexture().bind(TexU_u);
    _v.getTexture().bind(TexU_v);
    
    //perform advect (using the velocity forces to move around the densities)
    renderToGPUBuffer(d);
    
    //clean up
    _u.getTexture().unbind();
    _v.getTexture().unbind();
    AddVectShader.unbind();
    
    //adjust boundaries
    setBnd(d, b);
}

void FluidModelGLSL::project(GPUBuffer& _u, GPUBuffer& _v, GPUBuffer& _p, GPUBuffer& _div)
{
    //Texture unit definitions
    const int TexU_u = 0;
    const int TexU_v = 1;
    const int TexU_p = 2;

    //_p[IX(i, j)] = 0;
    initField(_p, 0);

    //Bind shader for Step 1 and pass variables as uniforms
    ProjectShader_Step1.bind();
    ProjectShader_Step1.uniform("u", TexU_u);
    ProjectShader_Step1.uniform("v", TexU_v);
    ProjectShader_Step1.uniform("OneNStep", OneNStep);
    ProjectShader_Step1.uniform("N", (float) N);
    
    //Bind textures of FBOs to the above-mentioned texture units
    _u.getTexture().bind(TexU_u);
    _v.getTexture().bind(TexU_v);
    
    //perform _div[IX(i, j)] = -0.5f * (_u[IX(i+1, j)] - _u[IX(i-1, j)] + _v[IX(i, j + 1)] - _v[IX(i, j - 1)]) / N;
    renderToGPUBuffer(_div);
    
    //clean up Step 1
    _u.getTexture().unbind();
    _v.getTexture().unbind();
    ProjectShader_Step1.unbind();
    
    //go on with Jos' algorithm
    setBnd(_div, 0);
    setBnd(_p, 0);    
    linSolve(_p, _div, 0, 1, 4);

    //In Step 2, a "_u... -= _u..." operation is necessary, so
    //copy _u to WorkBuffer since sampling and rendering to the same texture is not allowed/undefined/driver dependend in GLSL
    _u.blitTo(WorkBuffer, _u.getBounds(), WorkBuffer.getBounds());

    //Bind shader for Step 2 and pass variables as uniforms
    ProjectShader_Step2.bind();
    ProjectShader_Step2.uniform("u0", TexU_u);
    ProjectShader_Step2.uniform("p", TexU_p);
    ProjectShader_Step2.uniform("OneNStep", OneNStep);
    ProjectShader_Step2.uniform("N", (float) N);
    
    //Bind texture of FBO to the above-mentioned texture unit
    WorkBuffer.getTexture().bind(TexU_u);
    _p.getTexture().bind(TexU_p);
    
    //perform _u[IX(i, j)] -= 0.5f * N * (_p[IX(i + 1, j)] - _p[IX(i - 1, j)]);
    renderToGPUBuffer(_u);
    
    //clean up Step 2
    WorkBuffer.getTexture().unbind();
    _p.getTexture().unbind();
    ProjectShader_Step2.unbind();
    
    //In Step 3, a "_v... -= _v..." operation is necessary, so
    //copy _v to WorkBuffer since sampling and rendering to the same texture is not allowed/undefined/driver dependend in GLSL
    _v.blitTo(WorkBuffer, _v.getBounds(), WorkBuffer.getBounds());

    //Bind shader for Step 3 and pass variables as uniforms
    ProjectShader_Step3.bind();
    ProjectShader_Step3.uniform("v0", TexU_v);
    ProjectShader_Step3.uniform("p", TexU_p);
    ProjectShader_Step3.uniform("OneNStep", OneNStep);
    ProjectShader_Step3.uniform("N", (float) N);
    
    //Bind texture of FPO to the above-mentioned texture unit
    WorkBuffer.getTexture().bind(TexU_v);
    _p.getTexture().bind(TexU_p);
    
    //perform _v[IX(i, j)] -= 0.5f * N * (_p[IX(i, j + 1)] - _p[IX(i, j - 1)]);
    renderToGPUBuffer(_v);
    
    //clean up Step 3
    WorkBuffer.getTexture().unbind();
    _p.getTexture().unbind();
    ProjectShader_Step3.unbind();
    
    //finish Jos' algorithm
    setBnd(_u, 1);
    setBnd(_v, 2);
}

void FluidModelGLSL::calculateNextIterationForDensity()
{
    //calling syntax of all three functions is (target, source, ...), that means
    //means the output of the previous function is the input to the next one
    //modifier functions "force adders" add it always to ..._prev
    
    addSource(Density, Density_prev);
    diffuse(Density_prev, Density, 0, Diff);
    advect(Density, Density_prev, 0, u, v);
}

void FluidModelGLSL::calculateNextIterationForVelocity()
{
    addSource(u, u_prev);
    addSource(v, v_prev);
    
    diffuse(u_prev, u, 1, Viscosity);
    diffuse(v_prev, v, 2, Viscosity);
    
    project(u_prev, v_prev, u, v);
    
    advect(u, u_prev, 1, u_prev, v_prev);
    advect(v, v_prev, 2, u_prev, v_prev);
    
    project(u, v, u_prev, v_prev);
}

void FluidModelGLSL::iterateModel()
{  
    calculateNextIterationForVelocity();
    calculateNextIterationForDensity();
    
    initField(u_prev);
    initField(v_prev);   
    initField(Density_prev);
}

void FluidModelGLSL::addCircularForceArea(GPUBuffer& field, const int x, const int y, float s, float r)
{
    //copy field to WorkBuffer since sampling and rendering to the same texture is not allowed/undefined/driver dependend in GLSL
    field.blitTo(WorkBuffer, field.getBounds(), WorkBuffer.getBounds());
    
    //Texture unit definitions
    const int TexU_field0 = 0;
    
    //Bind shader and pass variables as uniforms
    AddForceShader_Circular.bind();
    AddForceShader_Circular.uniform("field0", TexU_field0);
    AddForceShader_Circular.uniform("x_center", (float) x * OneNStep);
    AddForceShader_Circular.uniform("y_center", (float) y * OneNStep);
    AddForceShader_Circular.uniform("s", s);
    AddForceShader_Circular.uniform("r", r * OneNStep);
        
    //Bind texture of WorkBuffer to the appropriate texture unit
    WorkBuffer.getTexture().bind(TexU_field0);
    
    //perform the actual force adding calculation
    renderToGPUBuffer(field);
    
    //clean up
    WorkBuffer.getTexture().unbind();
    AddForceShader_Circular.unbind();
}

void FluidModelGLSL::addCircularDensity(unsigned int x, unsigned int y, float s, float r)
{
    addCircularForceArea(Density_prev, x, y, s, r);
}

void FluidModelGLSL::addCircularVelocity(unsigned int x, unsigned int y, float sx, float sy, float r)
{
    addCircularForceArea(u_prev, x, y, sx, r);
    addCircularForceArea(v_prev, x, y, sy, r);
}
