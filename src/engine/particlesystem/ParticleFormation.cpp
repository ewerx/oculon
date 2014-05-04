//
//  ParticleFormation.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-03.
//
//

#include "ParticleFormation.h"
#include "cinder/Rand.h"
#include "cinder/Surface.h"

using namespace std;
using namespace ci;

#pragma mark - Base Formation

ParticleFormation::ParticleFormation(const std::string& name,
                  ci::gl::Texture posTex,
                  ci::gl::Texture velTex,
                  ci::gl::Texture dataTex)
: mName(name)
, mPositionTex(posTex)
, mVelocityTex(velTex)
, mDataTex(dataTex)
{
}

ParticleFormation::ParticleFormation(const std::string &name,
                                     const int fboSize,
                                     std::vector<ci::Vec4f> &positions,
                                     std::vector<ci::Vec4f> &velocities,
                                     std::vector<ci::Vec4f> &data)
: mName(name)
{
    // setup the framebuffers
    // bufSize x bufSize
    Surface32f posSurface = Surface32f(fboSize,fboSize,true);
	Surface32f velSurface = Surface32f(fboSize,fboSize,true);
	Surface32f dataSurface = Surface32f(fboSize,fboSize,true);
    
    Surface32f::Iter surfaceIter = posSurface.getIter();
    vector<Vec4f>::iterator posIter = positions.begin();
    vector<Vec4f>::iterator velIter = velocities.begin();
    vector<Vec4f>::iterator dataIter = data.begin();
    
    // encode the values as colors
    while(surfaceIter.line())
	{
		while(surfaceIter.pixel())
		{
            ColorAf posPixel = ColorAf( (*posIter).x, (*posIter).y, (*posIter).z, (*posIter).w );
            posSurface.setPixel(surfaceIter.getPos(), posPixel);
            posIter++;
            
            ColorAf velPixel = ColorAf( (*velIter).x, (*velIter).y, (*velIter).z, (*velIter).w );
            velSurface.setPixel(surfaceIter.getPos(), velPixel);
            velIter++;
            
            ColorAf dataPixel = ColorAf( (*dataIter).x, (*dataIter).y, (*dataIter).z, (*dataIter).w );
            dataSurface.setPixel(surfaceIter.getPos(), dataPixel);
            dataIter++;
        }
    }
    
    setTextures(posSurface, velSurface, dataSurface);
}

ParticleFormation::ParticleFormation(const std::string& name)
: mName(name)
{
}

ParticleFormation::~ParticleFormation()
{
}

void ParticleFormation::setTextures(ci::Surface32f &posSurface,
                                    ci::Surface32f &velSurface,
                                    ci::Surface32f &dataSurface)
{
    gl::Texture::Format format;
    format.setInternalFormat( GL_RGBA32F_ARB );
    
    mPositionTex = gl::Texture(posSurface, format);
    mPositionTex.setWrap( GL_REPEAT, GL_REPEAT );
    mPositionTex.setMinFilter( GL_NEAREST );
    mPositionTex.setMagFilter( GL_NEAREST );
    
    mVelocityTex = gl::Texture(velSurface, format);
    mVelocityTex.setWrap( GL_REPEAT, GL_REPEAT );
    mVelocityTex.setMinFilter( GL_NEAREST );
    mVelocityTex.setMagFilter( GL_NEAREST );
    
    mDataTex = gl::Texture(dataSurface, format);
    mDataTex.setWrap( GL_REPEAT, GL_REPEAT );
    mDataTex.setMinFilter( GL_NEAREST );
    mDataTex.setMagFilter( GL_NEAREST );
}

#pragma mark - Random Formation

RandomFormation::RandomFormation(const int fboSize)
: RandomFormation(fboSize,
                  Vec4f::zero(), Vec4f::one(),
                  Vec4f::zero(), Vec4f::one(),
                  Vec4f::zero(), Vec4f::one())
{
}

RandomFormation::RandomFormation(const int fboSize,
                                 const ci::Vec4f& posMin, const ci::Vec4f& posMax,
                                 const ci::Vec4f& velMin, const ci::Vec4f& velMax,
                                 const ci::Vec4f& dataMin, const ci::Vec4f& dataMax)
: ParticleFormation("random")
{
    // setup the framebuffers
    // bufSize x bufSize
    Surface32f posSurface = Surface32f(fboSize,fboSize,true);
	Surface32f velSurface = Surface32f(fboSize,fboSize,true);
	Surface32f dataSurface = Surface32f(fboSize,fboSize,true);
    
    Surface32f::Iter surfaceIter = posSurface.getIter();
    
    // random
    while(surfaceIter.line())
	{
		while(surfaceIter.pixel())
		{
            // position + mass
            float x = Rand::randFloat(posMin.x,posMax.x);
            float y = Rand::randFloat(posMin.y,posMax.y);
            float z = Rand::randFloat(posMin.z,posMax.z);
            float w = Rand::randFloat(posMin.w,posMax.w);
            posSurface.setPixel(surfaceIter.getPos(), ColorAf(x,y,z,w));
            
            // velocity + age
            float vx = Rand::randFloat(velMin.x,velMax.x);
            float vy = Rand::randFloat(velMin.y,velMax.y);
            float vz = Rand::randFloat(velMin.z,velMax.z);
            float vw = Rand::randFloat(velMin.w,velMax.w);
            velSurface.setPixel(surfaceIter.getPos(), ColorAf(vx,vy,vz,vw));
            
            // extra data
            float dx = Rand::randFloat(dataMin.x,dataMax.x);
            float dy = Rand::randFloat(dataMin.y,dataMax.y);
            float dz = Rand::randFloat(dataMin.z,dataMax.z);
            float dw = Rand::randFloat(dataMin.w,dataMax.w);
            dataSurface.setPixel(surfaceIter.getPos(), ColorAf(dx,dy,dz,dw));
        }
    }
    
    setTextures(posSurface, velSurface, dataSurface);
}
