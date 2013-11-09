/*
 ALL CREDIT for this code goes to KYLE MCDONALD and his original OF version
 posted here http://www.openframeworks.cc/forum/viewtopic.php?f=12&t=2860
 */

#pragma once
#include "BinnedParticle.h"
#include "cinder/gl/Texture.h"

class Scene;

#define DRAW_FORCES
#define USE_INVSQRT

namespace bps 
{

class ParticleSystem 
{
protected:
	float timeStep;
    std::vector<Particle> particles;
	std::vector< std::vector<Particle*> > bins;
	int width, height, k, xBins, yBins, binSize;
    const ci::ColorAf* mForceColor;
    
    Scene* mScene;
    
    ci::gl::Texture mParticleTexture;

public:
	ParticleSystem();

	void setup(int width, int height, int k, Scene* scene);
	void setTimeStep(float timeStep);

	void add(Particle particle);
	std::vector<Particle*> getNeighbors(Particle& particle, float radius);
	std::vector<Particle*> getNeighbors(float x, float y, float radius);
	std::vector<Particle*> getRegion(unsigned minX, unsigned minY, unsigned maxX, unsigned maxY);
	unsigned size() const;
	Particle& operator[](unsigned i);

	void setupForces();
	void addRepulsionForce(const Particle& particle, float radius, float scale);
	void addRepulsionForce(float x, float y, float radius, float scaleX, float scaleY);
	void addAttractionForce(const Particle& particle, float radius, float scale);
	void addAttractionForce(float x, float y, float radius, float scale);
	void addForce(const Particle& particle, float radius, float scale);
	void addForce(float x, float y, float radius, float scaleX, float scaleY);
	void update();

	void draw(const ci::ColorAf& color, float radius);
    
    void setForceColor(const ci::ColorAf* color) { mForceColor = color; }
};

}