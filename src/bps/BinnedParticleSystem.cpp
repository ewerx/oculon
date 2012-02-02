/*
 ALL CREDIT for this code goes to KYLE MCDONALD and his original OF version
 posted here http://www.openframeworks.cc/forum/viewtopic.php?f=12&t=2860
 */

#include "BinnedParticleSystem.h"

using namespace bps;

ParticleSystem::ParticleSystem() :
	timeStep(1) {
}

void ParticleSystem::setup(int width, int height, int k) {
	this->width = width;
	this->height = height;
	this->k = k;
	binSize = 1 << k;
	xBins = (int) ceilf((float) width / (float) binSize);
	yBins = (int) ceilf((float) height / (float) binSize);
	bins.resize(xBins * yBins);
    particles.clear();
}

void ParticleSystem::setTimeStep(float timeStep) {
	this->timeStep = timeStep;
}

void ParticleSystem::add(bps::Particle particle) {
	particles.push_back(particle);
}

unsigned ParticleSystem::size() const {
	return particles.size();
}

bps::Particle& ParticleSystem::operator[](unsigned i) {
	return particles[i];
}

vector<bps::Particle*> ParticleSystem::getNeighbors(bps::Particle& particle, float radius) {
	return getNeighbors(particle.x, particle.y, radius);
}

vector<bps::Particle*> ParticleSystem::getNeighbors(float x, float y, float radius) {
	vector<bps::Particle*> region = getRegion(
		(int) (x - radius),
		(int) (y - radius),
		(int) (x + radius),
		(int) (y + radius));
	vector<bps::Particle*> neighbors;
	int n = region.size();
	float xd, yd, rsq, maxrsq;
	maxrsq = radius * radius;
	for(int i = 0; i < n; i++) {
		bps::Particle& cur = *region[i];
		xd = cur.x - x;
		yd = cur.y - y;
		rsq = xd * xd + yd * yd;
		if(rsq < maxrsq)
			neighbors.push_back(region[i]);
	}
	return neighbors;
}

vector<bps::Particle*> ParticleSystem::getRegion(unsigned minX, unsigned minY, unsigned maxX, unsigned maxY) {
	vector<bps::Particle*> region;
	back_insert_iterator< vector<bps::Particle*> > back = back_inserter(region);
	unsigned minXBin = minX >> k;
	unsigned maxXBin = maxX >> k;
	unsigned minYBin = minY >> k;
	unsigned maxYBin = maxY >> k;
	maxXBin++;
	maxYBin++;
	if(maxXBin > xBins)
		maxXBin = xBins;
	if(maxYBin > yBins)
		maxYBin = yBins;
	for(int y = minYBin; y < maxYBin; y++) {
		for(int x = minXBin; x < maxXBin; x++) {
			vector<bps::Particle*>& cur = bins[y * xBins + x];
			copy(cur.begin(), cur.end(), back);
		}
	}
	return region;
}

void ParticleSystem::setupForces() {
	int n = bins.size();
	for(int i = 0; i < n; i++) {
		bins[i].clear();
	}
	n = particles.size();
	unsigned xBin, yBin, bin;
	for(int i = 0; i < n; i++) {
		bps::Particle& cur = particles[i];
		cur.resetForce();
		xBin = ((unsigned) cur.x) >> k;
		yBin = ((unsigned) cur.y) >> k;
		bin = yBin * xBins + xBin;
		if(xBin < xBins && yBin < yBins)
			bins[bin].push_back(&cur);
	}
}

void ParticleSystem::addRepulsionForce(const bps::Particle& particle, float radius, float scale) {
	addRepulsionForce(particle.x, particle.y, radius, scale, scale);
}

void ParticleSystem::addRepulsionForce(float x, float y, float radius, float scaleX, float scaleY) {
	addForce(x, y, radius, scaleX, scaleY);
}

void ParticleSystem::addAttractionForce(const bps::Particle& particle, float radius, float scale) {
	addAttractionForce(particle.x, particle.y, radius, scale);
}

void ParticleSystem::addAttractionForce(float x, float y, float radius, float scale) {
	addForce(x, y, radius, -scale, -scale);
}

void ParticleSystem::addForce(const bps::Particle& particle, float radius, float scale) {
	addForce(particle.x, particle.y, radius, -scale, -scale);
}

void ParticleSystem::addForce(float targetX, float targetY, float radius, float scaleX, float scaleY) {
	float minX = targetX - radius;
	float minY = targetY - radius;
	float maxX = targetX + radius;
	float maxY = targetY + radius;
	if(minX < 0)
		minX = 0;
	if(minY < 0)
		minY = 0;
	unsigned minXBin = ((unsigned) minX) >> k;
	unsigned minYBin = ((unsigned) minY) >> k;
	unsigned maxXBin = ((unsigned) maxX) >> k;
	unsigned maxYBin = ((unsigned) maxY) >> k;
	maxXBin++;
	maxYBin++;
	if(maxXBin > xBins)
		maxXBin = xBins;
	if(maxYBin > yBins)
		maxYBin = yBins;
	float xd, yd, length, maxrsq;
	#ifdef USE_INVSQRT
	float xhalf;
	int lengthi;
	#else
	float effect;
	#endif
	maxrsq = radius * radius;
	int x,y;
	for(y = minYBin; y < maxYBin; y++) {
		for(x = minXBin; x < maxXBin; x++) {
			vector<bps::Particle*>& curBin = bins[y * xBins + x];
			int n = curBin.size();
			for(int i = 0; i < n; i++) {
				bps::Particle& curParticle = *(curBin[i]);
//				As suggested by Andrew Bell
//				if(curParticle.x > minX && curParticle.x < maxX &&
//					curParticle.y > minY && curParticle.y < maxY) {
					xd = curParticle.x - targetX;
					yd = curParticle.y - targetY;
					length = xd * xd + yd * yd;
					if(length > 0 && length < maxrsq) {
						#ifdef DRAW_FORCES
							glVertex2f(targetX, targetY);
							glVertex2f(curParticle.x, curParticle.y);
						#endif
						#ifdef USE_INVSQRT
							xhalf = 0.5f * length;
							lengthi = *(int*) &length;
							lengthi = 0x5f3759df - (lengthi >> 1);
							length = *(float*) &lengthi;
							length *= 1.5f - xhalf * length * length;
							xd *= length;
							yd *= length;
							length *= radius;
							length = 1 / length;
							length = (1 - length);
							xd *= (length * scaleX);
							yd *= (length * scaleY);
							curParticle.xf += xd;
							curParticle.yf += yd;
						#else
							length = sqrtf(length);
							xd /= length;
							yd /= length;
							effect = (1 - (length / radius)) * scale;
							curParticle.xf += xd * effect;
							curParticle.yf += yd * effect;
						#endif
					}
//				}
			}
		}
	}
}

void ParticleSystem::update() {
	int n = particles.size();
	for(int i = 0; i < n; i++)
		particles[i].updatePosition(timeStep);
}

void ParticleSystem::draw() {
	int n = particles.size();
	glBegin(GL_POINTS);
	for(int i = 0; i < n; i++)
		particles[i].draw();
	glEnd();
}

