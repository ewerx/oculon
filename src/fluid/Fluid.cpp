//
//  Fluid.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 1/12/2014.
//
//

#include "Fluid.h"
#include "OculonApp.h"
#include "Interface.h"
#include "fluidmodel_gpu_glsl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static const int GRADIENT_COLORCOUNT = 2048;

#pragma mark - Construction

Fluid::Fluid()
: Scene("fluid")
, mFluidModel(NULL)
{
}

Fluid::~Fluid()
{
}

#pragma mark - Setup

void Fluid::setup()
{
    Scene::setup();
    
    mAudioInputHandler.setup(false);
    
    // params
    mViscosity      = 0.00001f;
    mDiffusion      = 0.00002f;
    mQuadraticSize  = 200;
    mSimulationRate = 0.1f;
    mEnableLighting = true;
    
    mDrawShader = loadFragShader("frag_draw.glsl");
    
    // gradient
    gl::Texture::Format format;
    format.setInternalFormat(GL_RGBA32F_ARB);
    mGradientTexture = gl::Texture(GRADIENT_COLORCOUNT, 1, format);
    
    updateGradient(3);
    
    resetFluidModel();
    resetQuadraticSize();
}

void Fluid::resetFluidModel()
{
    if (mFluidModel) {
        delete mFluidModel;
    }
    mFluidModel = new FluidModelGLSL(mQuadraticSize, mViscosity, mSimulationRate, mDiffusion, mApp);
}

void Fluid::resetQuadraticSize()
{
    //change "N" inside the model
    mFluidModel->setQuadraticSize(mQuadraticSize);

    //in float coords, the width or height of one pixel inside the Density texture is 1/(N-1)
    //because for drawing we are only regaring the 1..N interval (in contrast to rendering inside the FluidModel)
    mQuadraticPixelSize = 1.0f / ((float) mFluidModel->getQuadraticSize() - 1.0f);
}

void Fluid::updateGradient(int colorScheme)
{
    mGradientMaker.makeDefaultGradient(colorScheme);
    mGradientMaker.fillVectorOfColors(GRADIENT_COLORCOUNT, mGradientColors);

    //update gradient in VRAM by painting the gradient to a surface and then to a texture
    //using float colors is easier in GLSL, so convert them back to float
    Surface32f paintedgradient = Surface32f(GRADIENT_COLORCOUNT, 1, false, SurfaceChannelOrder::RGBA);
    Vec2i coord = Vec2i(0, 0);
    for (coord.x = 0; coord.x < GRADIENT_COLORCOUNT; coord.x++)
    {
        paintedgradient.setPixel(coord, mGradientColors[coord.x]);
    }
    mGradientTexture = gl::Texture(paintedgradient); //copy to VRAM
}

void Fluid::reset()
{
}

void Fluid::shutdown()
{
    delete mFluidModel;
}

#pragma mark - Interface

void Fluid::setupInterface()
{
    
    
    mAudioInputHandler.setupInterface(mInterface, mName);
}

void Fluid::handleMouseDown(const ci::app::MouseEvent &event)
{
    return;
    mPrevMousePos = event.getPos();
    if (event.isRightDown())
    {
        handleMouseDrag(event);
    }
}

void Fluid::handleMouseDrag(const ci::app::MouseEvent &event)
{
    return;
    Vec2i mousePos = event.getPos();
    Vec2i mouseVel = mousePos - mPrevMousePos;
    
    // TODO: make params...
    float BrushSize_Velocity       = 10;
    float BrushSize_Density        = 20;
    int MousePrecision_Velocity     = 4;
    
    //calculate a stretch factor of the window relative to the original canonical size of the surface
    //this is needed to scale down the mouse coordinates to the correct position in memory
    float x_stretch = (float) mApp->getViewportWidth() / (float) mQuadraticSize;
    float y_stretch = (float) mApp->getViewportHeight() / (float) mQuadraticSize;
    
    //downsize mouse cursor position to the fluid grid and remember that it runs from [1..N]
    int x_mapping = (int) ((float) mousePos.x / ((float) x_stretch)) + 1;
    int y_mapping = (int) ((float) mousePos.y / ((float) y_stretch)) + 1;
    
    //add density (right-click)
    //use a static "strength" for the density, because it reveals a quite natural feeling
    if (event.isRightDown())
    {
        mFluidModel->addCircularDensity(x_mapping, y_mapping, 0.5f, BrushSize_Density);
    }
    
    //add force (left-click)
    //the strength is a vector resulting from the mouse velocity
    if (event.isLeftDown())
    {
        if (MousePrecision_Velocity > 0)
        {
            //The Velocity Precision interaction parameter MousePrecision_Velocity damps in a non-linear fashion:
            //the faster the user moves the mouse, the less damping effect is there, i.e. the more direct all
            //mouse commands are executed. In other words: if the user moves the mouse very slowly, we are adding
            //very preceise velocities to the system. This is achieved by using a moved (1/x) family function.
            //And since lim(1/x)=>0, the limit of the used (Precision / x) + 1 is 1, i.e. the division which
            //"damps" the movement is having no effect any more for very fast movements.
            //MouseVel is an integer, therefore we do not have the unwanted "exorbitant" effects
            //between 0..1 that (1/x) would normaly have.
            
            float xf = mouseVel.x != 0 ? (((float) MousePrecision_Velocity / (float) abs(mouseVel.x)) + 1.0f) : 1.0f;
            float yf = mouseVel.y != 0 ? (((float) MousePrecision_Velocity / (float) abs(mouseVel.y)) + 1.0f) : 1.0f;
            
            mFluidModel->addCircularVelocity(x_mapping, y_mapping, ((float) mouseVel.x) / xf, ((float) mouseVel.y) / yf, BrushSize_Velocity);
        }
        else
        {
            mFluidModel->addCircularVelocity(x_mapping, y_mapping, mouseVel.x, mouseVel.y, BrushSize_Velocity);
        }
    }
    
    mPrevMousePos = mousePos;
}

#pragma mark - Update

void Fluid::update(double dt)
{
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    {
        int x = Rand::randInt(1,mQuadraticSize);
        int y = Rand::randInt(1,mQuadraticSize);
        mFluidModel->addCircularDensity(x, y, 0.5f, Rand::randInt(1,40));
    }
    {
        int x = Rand::randInt(1,mQuadraticSize);
        int y = Rand::randInt(1,mQuadraticSize);
        int sx = Rand::randInt(1,mQuadraticSize);
        int sy = Rand::randInt(1,mQuadraticSize);
        mFluidModel->addCircularVelocity(x, y, sx, sy, Rand::randInt(1,40));
    }
    
    mFluidModel->iterateModel();
    
    //MouseEvent event(nullptr, MouseEvent::LEFT_DOWN, Rand::randInt(mApp->getViewportWidth()), Rand::randInt(mApp->getViewportHeight()), 0, 0.0f, 0);
    //handleMouseDrag(event);
    

    
    Scene::update(dt);
}


#pragma mark - Draw

void Fluid::draw()
{
    gl::pushMatrices();
    glPushAttrib( GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT );
    
    gl::enableAlphaBlending();
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    gl::setMatricesWindow( mApp->getViewportSize() );
    gl::setViewport( mApp->getViewportBounds() );
    
    drawFluidModel();
    
    glPopAttrib();
    gl::popMatrices();
}

void Fluid::drawFluidModel()
{
    gl::enable( GL_TEXTURE_2D );
    
    //access the fluid model's current density state
    const gl::Texture& densityTexture = ((FluidModelGLSL*) mFluidModel)->getDensity();
    
    //the texture coordinates are 1..N whereas the draw coordinates are the window's size
    const Rectf srcCoords = ((FluidModelGLSL*) mFluidModel)->getDensityTextureCoordinates();
    
    //texture unit definitions
    const int TexU_Density = 0;
    const int TexU_GradientColors = 1;
    
    //Bind GLSL shader and generate/pass all variables
    mDrawShader.bind();
    mDrawShader.uniform("Density", TexU_Density);
    mDrawShader.uniform("GradientColors", TexU_GradientColors);
    mDrawShader.uniform("LightingFactor", 2.0f);
    mDrawShader.uniform("LightingPower", 100000.0f);
    mDrawShader.uniform("OnePixel", mQuadraticPixelSize);
    mDrawShader.uniform("Show3DLighting", mEnableLighting);
    
    //bind textures to the texture unit numbers as defined in the above-mentioned uniform statements
    densityTexture.bind(TexU_Density);
    mGradientTexture.bind(TexU_GradientColors);
    
    //fill the whole window
    const Rectf destRect = Rectf(mApp->getViewportBounds());
    
    //draw the texture by using two texture mapped triangles
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
    
    texCoords[0*2+0] = srcCoords.getX2(); texCoords[0*2+1] = srcCoords.getY1();
    texCoords[1*2+0] = srcCoords.getX1(); texCoords[1*2+1] = srcCoords.getY1();
    texCoords[2*2+0] = srcCoords.getX2(); texCoords[2*2+1] = srcCoords.getY2();
    texCoords[3*2+0] = srcCoords.getX1(); texCoords[3*2+1] = srcCoords.getY2();
    
    //actually draw it
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    //clean up
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    
//    // Draw shader output
//    gl::enable( GL_TEXTURE_2D );
//    gl::color( Colorf::white() );
//    gl::begin( GL_TRIANGLES );
//    
//    // Define quad vertices
//    const Area& bounds = mApp->getViewportBounds();
//    
//    Vec2f vert0( (float)bounds.x1, (float)bounds.y1 );
//    Vec2f vert1( (float)bounds.x2, (float)bounds.y1 );
//    Vec2f vert2( (float)bounds.x1, (float)bounds.y2 );
//    Vec2f vert3( (float)bounds.x2, (float)bounds.y2 );
//    
//    // Define quad texture coordinates
//    Vec2f uv0( 0.0f, 0.0f );
//    Vec2f uv1( 1.0f, 0.0f );
//    Vec2f uv2( 0.0f, 1.0f );
//    Vec2f uv3( 1.0f, 1.0f );
//    
//    // Draw quad (two triangles)
//    gl::texCoord( uv0 );
//    gl::vertex( vert0 );
//    gl::texCoord( uv2 );
//    gl::vertex( vert2 );
//    gl::texCoord( uv1 );
//    gl::vertex( vert1 );
//    
//    gl::texCoord( uv1 );
//    gl::vertex( vert1 );
//    gl::texCoord( uv2 );
//    gl::vertex( vert2 );
//    gl::texCoord( uv3 );
//    gl::vertex( vert3 );
//    
//    gl::end();
    
    densityTexture.unbind();
    mGradientTexture.unbind();
    mDrawShader.unbind();
}

void Fluid::drawDebug()
{
    Scene::drawDebug();
    
    gl::pushMatrices();
    glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
    gl::enable( GL_TEXTURE_2D );
    const Vec2f& windowSize( getWindowSize() );
    gl::setMatricesWindow( windowSize );
    
    const float size = 80.0f;
    const float paddingX = 20.0f;
    const float paddingY = 240.0f;
    
    const gl::Texture& densityTexture = ((FluidModelGLSL*) mFluidModel)->getDensity();
    Rectf preview( windowSize.x - (size+paddingX), windowSize.y - (size+paddingY), windowSize.x-paddingX, windowSize.y - paddingY );
    gl::draw( densityTexture, preview );
    
    Rectf preview2 = preview - Vec2f(size+paddingX, 0.0f);
    gl::draw( mGradientTexture, preview2 );
    
    glPopAttrib();
    gl::popMatrices();
}
